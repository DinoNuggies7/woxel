#include "entity.h"
#include "input.h"
#include "world.h"
#include "blocks.h"
#include "menu.h"
#include "sound.h"

#include <malloc.h>
#include <raymath.h>

Vector2 get_movedir(Camera3D);
Vector3 get_look_block(Camera3D);
void place_block(Camera3D cam, int block);

void E_PLAYER_INIT(Entity* this) {
	this->data = malloc(sizeof(Camera3D) + sizeof(int) + sizeof(bool) + sizeof(Menu));
	this->var = malloc(sizeof(void*) * 4);
	this->var[0] = this->data;
	this->var[1] = this->data + sizeof(Camera3D);
	this->var[2] = this->data + sizeof(Camera3D) + sizeof(int);
	this->var[3] = this->data + sizeof(Camera3D) + sizeof(int) + sizeof(bool);

	Camera3D* cam = this->var[0];
	cam->projection = CAMERA_PERSPECTIVE;
	cam->position = (Vector3){0, 0, 0};
	cam->target = (Vector3){0, 0, -1};
	cam->up = (Vector3){0, 1, 0};
	cam->fovy = 90;

	int* block = this->var[1];
	*block = B_DIRT;

	bool* in_menu = this->var[2];
	*in_menu = false;

	Menu* menu = this->var[3];
	*menu = spawn_menu(5, "Pause");
	set_menu_option(menu, 0, "Resume");
	set_menu_option(menu, 1, "Save World");
	set_menu_option(menu, 2, "Load World");
	set_menu_option(menu, 3, "Fullscreen");
	set_menu_option(menu, 4, "Exit");
	update_menu(menu);

	this->pos.x = world.w * 0.5;
	this->pos.z = world.l * 0.5;
	this->pos.y = 13;

	this->vel.x = this->vel.y = this->vel.z = 0;

	this->size.x = this->size.z = 0.2;
	this->size.y = 1.8;

	cam->position = Vector3Add(cam->position, this->pos);
	cam->position.y += 1.5;
	cam->target = Vector3Add(cam->target, cam->position);
}

void E_PLAYER_TICK(Entity* this) {
	static bool flying;
	static bool grounded;
	static int step_sound;
	static float step_sound_timer;
	static bool fullscreen;

	const float speed = 0.1;
	const float jump_speed = 0.17;
	const float gravity = 0.01;
	const float terminal_vel = -1;
	const float step_sound_delay = 2.3;

	Camera3D* cam = this->var[0];
	int* block = this->var[1];
	bool* in_menu = this->var[2];
	Menu* menu = this->var[3];

	Vector2 dir = get_movedir(*cam);
	this->vel.x = dir.x * speed;
	this->vel.z = dir.y * speed;

	if (input.crouch) {
		this->vel.x *= 0.5;
		this->vel.z *= 0.5;
	}

	if (!flying) {
		if (this->vel.y > terminal_vel)
			this->vel.y -= gravity;

		if (get_block(this->pos.x - this->size.x, this->pos.y + this->vel.y, this->pos.z - this->size.z) > B_AIR
		 || get_block(this->pos.x - this->size.x, this->pos.y + this->vel.y, this->pos.z + this->size.z) > B_AIR
		 || get_block(this->pos.x + this->size.x, this->pos.y + this->vel.y, this->pos.z - this->size.z) > B_AIR
		 || get_block(this->pos.x + this->size.x, this->pos.y + this->vel.y, this->pos.z + this->size.z) > B_AIR)
			grounded = true;

		entity_collision(this);

		if (input.jump && grounded) {
			this->vel.y = jump_speed;
			grounded = false;
			PlaySound(sound[S_STEP0 + GetRandomValue(0, 3)]);
		}

		if (input.crouch && grounded) {
			if (get_block(this->pos.x + this->vel.x - this->size.x, this->pos.y - 1, this->pos.z - this->size.z) == B_AIR
			 && get_block(this->pos.x + this->vel.x - this->size.x, this->pos.y - 1, this->pos.z + this->size.z) == B_AIR
			 && get_block(this->pos.x + this->vel.x + this->size.x, this->pos.y - 1, this->pos.z - this->size.z) == B_AIR
			 && get_block(this->pos.x + this->vel.x + this->size.x, this->pos.y - 1, this->pos.z + this->size.z) == B_AIR)
				this->vel.x = 0;

			if (get_block(this->pos.x - this->size.x, this->pos.y - 1, this->pos.z + this->vel.z - this->size.z) == B_AIR
			 && get_block(this->pos.x - this->size.x, this->pos.y - 1, this->pos.z + this->vel.z + this->size.z) == B_AIR
			 && get_block(this->pos.x + this->size.x, this->pos.y - 1, this->pos.z + this->vel.z - this->size.z) == B_AIR
			 && get_block(this->pos.x + this->size.x, this->pos.y - 1, this->pos.z + this->vel.z + this->size.z) == B_AIR)
				this->vel.z = 0;
		}

		float actual_speed = fabsf(this->vel.x) + fabsf(this->vel.z);
		if (grounded && actual_speed > 0) {
			if (!IsSoundPlaying(sound[S_STEP0 + step_sound])) {
				if (step_sound_timer >= step_sound_delay) {
					step_sound = GetRandomValue(0, 3);
					PlaySound(sound[S_STEP0 + step_sound]);
					step_sound_timer = 0;
				}
			}
			step_sound_timer += actual_speed;
		}
		else
			step_sound_timer = step_sound_delay;
	}
	else {
		if (input.jump) {
			input.jump = false;
			this->vel.y = speed;
		}
		else if (input.crouch) {
			input.crouch = false;
			this->vel.y = -speed;
		}
		else
			this->vel.y = 0;
	}

	if (!*in_menu) {
		if (input.hit) {
			input.hit = false;
			Vector3 v = get_look_block(*cam);
			if (v.x != 0 || v.y != 0 || v.z != 0) {
				set_block(v.x, v.y, v.z, B_AIR);
				PlaySound(sound[S_TICK]);
			}
		}
		if (input.use) {
			input.use = false;
			place_block(*cam, *block);
		}
	}
	else {
		menu_tick(menu);
		if (menu->option[0]) {
			menu->option[0] = false;
			*in_menu = false;
		}
		if (menu->option[1]) {
			menu->option[1] = false;
			save_world("world.wwf");
			PlaySound(sound[S_DENY]);
		}
		if (menu->option[2]) {
			menu->option[2] = false;
			destroy_world();
			load_world("world.wwf");
			PlaySound(sound[S_DENY]);
		}
		if (menu->option[3]) {
			menu->option[3] = false;
			fullscreen = !fullscreen;
			if (fullscreen) {
				SetWindowSize(GetMonitorWidth(GetCurrentMonitor()), GetMonitorHeight(GetCurrentMonitor()));
				ToggleFullscreen();
			}
			else {
				ToggleFullscreen();
				SetWindowSize(800, 600);
			}
			update_menu(menu);
		}
		if (menu->option[4]) {
			menu->option[4] = false;
			PlaySound(sound[S_BUTTON]);
			if (GetRandomValue(1, 69) == 1)
				PlaySound(sound[S_FIDDLESTICKS]);
		}
	}

	this->pos = Vector3Add(this->pos, this->vel);
	cam->position = Vector3Add(cam->position, this->vel);
	cam->target = Vector3Add(cam->target, this->vel);

	if (input.nextslot && *block < B_STONE) {
		input.nextslot = false;
		*block += 1;
		PlaySound(sound[S_SWITCH]);
	}

	if (input.prevslot && *block > B_DIRT)  {
		input.prevslot = false;
		*block -= 1;
		PlaySound(sound[S_SWITCH]);
	}

	if (input.inventory) {
		input.inventory = false;
		*in_menu = !*in_menu;
		PlaySound(sound[S_MOVESELECT]);
	}

	if (input.fly) {
		input.fly = false;
		flying = !flying;
		PlaySound(sound[S_DENY]);
	}
}

void __attribute__((constructor)) _construct_player() {
	ENTITY_INIT[E_PLAYER] = E_PLAYER_INIT;
	ENTITY_TICK[E_PLAYER] = E_PLAYER_TICK;
}

Vector2 get_movedir(Camera cam) {
	// bug: somehow walking between cardinal directions is ~1.4 times faster
	Vector3 lookdir = Vector3Normalize(Vector3Subtract(cam.target, cam.position));
	Vector2 movedir = Vector2Normalize((Vector2){lookdir.x, lookdir.z});
	Vector2 dir = {0, 0};
	if (input.forward) {
		dir.x += movedir.x;
		dir.y += movedir.y;
	}
	if (input.backward) {
		dir.x -= movedir.x;
		dir.y -= movedir.y;
	}
	if (input.left) {
		dir.x += movedir.y;
		dir.y -= movedir.x;
	}
	if (input.right) {
		dir.x -= movedir.y;
		dir.y += movedir.x;
	}
	return Vector2Normalize(dir);
}

Vector3 get_look_block(Camera3D cam) {
	Vector3 dir = Vector3Normalize(Vector3Subtract(cam.target, cam.position));
	for (float i = 0; i < 5; i += 0.1) {
		int x = cam.position.x + dir.x * i;
		int y = cam.position.y + dir.y * i;
		int z = cam.position.z + dir.z * i;
		if (get_block(x, y, z) > B_AIR)
			return (Vector3){x, y, z};
	}
	return (Vector3){0, 0, 0};
}

void place_block(Camera3D cam, int block) {
	Vector3 dir = Vector3Normalize(Vector3Subtract(cam.target, cam.position));
	for (float i = 0; i < 5; i += 0.1) {
		int x = cam.position.x + dir.x * i;
		int y = cam.position.y + dir.y * i;
		int z = cam.position.z + dir.z * i;
		if (get_block(x, y, z) > B_AIR) {
			x = cam.position.x + dir.x * (i - 0.1);
			y = cam.position.y + dir.y * (i - 0.1);
			z = cam.position.z + dir.z * (i - 0.1);
			set_block(x, y, z, block);
			PlaySound(sound[S_TICK]);
			return;
		}
	}
}
