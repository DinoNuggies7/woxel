#include "input.h"

#include <raylib.h>

struct Input input;

void set_binds() {
	input.key_forward   = KEY_W;
	input.key_backward  = KEY_S;
	input.key_left      = KEY_A;
	input.key_right     = KEY_D;
	input.key_jump      = KEY_SPACE;
	input.key_crouch    = KEY_LEFT_SHIFT;
	input.key_hit       = MOUSE_BUTTON_LEFT;
	input.key_use       = MOUSE_BUTTON_RIGHT;
	input.key_inventory = KEY_TAB;
	input.key_pause     = KEY_ESCAPE;
	input.key_nextslot  = KEY_E;
	input.key_prevslot  = KEY_Q;
	input.key_fly       = KEY_F;
	input.key_mconfirm  = KEY_ENTER;
	input.key_mup       = KEY_UP;
	input.key_mdown     = KEY_DOWN;
}

void get_input() {
	input.forward   = IsKeyDown(input.key_forward);
	input.backward  = IsKeyDown(input.key_backward);
	input.left      = IsKeyDown(input.key_left);
	input.right     = IsKeyDown(input.key_right);
	input.jump      = IsKeyDown(input.key_jump);
	input.crouch    = IsKeyDown(input.key_crouch);
	input.hit       = IsMouseButtonDown(input.key_hit) && input.hit_bool;
	input.use       = IsMouseButtonDown(input.key_use) && input.use_bool;
	input.inventory = IsKeyDown(input.key_inventory)   && input.inventory_bool;
	input.pause     = IsKeyDown(input.key_pause)       && input.pause_bool;
	input.nextslot  = IsKeyDown(input.key_nextslot)    && input.nextslot_bool;
	input.prevslot  = IsKeyDown(input.key_prevslot)    && input.prevslot_bool;
	input.fly       = IsKeyDown(input.key_fly)         && input.fly_bool;
	input.mconfirm  = IsKeyDown(input.key_mconfirm);
	input.mup       = IsKeyDown(input.key_mup)         && input.mup_bool;
	input.mdown     = IsKeyDown(input.key_mdown)       && input.mdown_bool;
	input.mdx       = GetMouseDelta().x;
	input.mdy       = GetMouseDelta().y;
}

void input_tick() {
	if (input.hit)       input.hit_bool       = false;
	if (input.use)       input.use_bool       = false;
	if (input.inventory) input.inventory_bool = false;
	if (input.pause)     input.pause_bool     = false;
	if (input.nextslot)  input.nextslot_bool  = false;
	if (input.prevslot)  input.prevslot_bool  = false;
	if (input.fly)       input.fly_bool       = false;
	if (input.mup)       input.mup_bool       = false;
	if (input.mdown)     input.mdown_bool     = false;
	if (IsMouseButtonUp(input.key_hit)) input.hit_bool       = true;
	if (IsMouseButtonUp(input.key_use)) input.use_bool       = true;
	if (IsKeyUp(input.key_inventory))   input.inventory_bool = true;
	if (IsKeyUp(input.key_pause))       input.pause_bool     = true;
	if (IsKeyUp(input.key_nextslot))    input.nextslot_bool  = true;
	if (IsKeyUp(input.key_prevslot))    input.prevslot_bool  = true;
	if (IsKeyUp(input.key_fly))         input.fly_bool       = true;
	if (IsKeyUp(input.key_mup))         input.mup_bool       = true;
	if (IsKeyUp(input.key_mdown))       input.mdown_bool     = true;
}
