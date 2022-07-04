#ifdef HW_RVL

#include <gccore.h>
#include <string.h>
#include <wiidrc/wiidrc.h>
#include "controller.h"
#include "../wiiSXconfig.h"

#define DRC_DEADZONE 0.078f
static unsigned int convertToPSRange(const int raw)
{
	// Convert raw from a value between [-1, 1].
	// It's easier to convert to another analog range this way.
	float converted = (float)(raw / 75.0f);

	if(converted > DRC_DEADZONE)
		converted = (converted-DRC_DEADZONE)*1.08f;
	else if(converted < -DRC_DEADZONE)
		converted = (converted+DRC_DEADZONE)*1.08f;
	else
		converted = 0;

	if (converted < -1.0f)
		converted = -1.0f;
	else if (converted > 1.0f)
		converted = 1.0f;

	float result = 0.0f;
	if (converted < 0.0f)
		result = (converted * 128.0f) - 128.0f;
	else
		result = (converted * 127.0f) + 127.0f;

	return result;
}

enum {
	L_STICK_AS_ANALOG = 1, R_STICK_AS_ANALOG = 2,
};

enum {
	L_STICK_L = 0x01 << 16,
	L_STICK_R = 0x02 << 16,
	L_STICK_U = 0x04 << 16,
	L_STICK_D = 0x08 << 16,
	R_STICK_L = 0x10 << 16,
	R_STICK_R = 0x20 << 16,
	R_STICK_U = 0x40 << 16,
	R_STICK_D = 0x80 << 16,
};

static button_t buttons[] = {
	{ 0, ~0, "None" },
	{ 1, WIIDRC_BUTTON_UP, "D-Up" },
	{ 2, WIIDRC_BUTTON_LEFT, "D-Left" },
	{ 3, WIIDRC_BUTTON_RIGHT, "D-Right" },
	{ 4, WIIDRC_BUTTON_DOWN, "D-Down" },
	{ 5, WIIDRC_BUTTON_L, "L" },
	{ 6, WIIDRC_BUTTON_R, "R" },
	{ 7, WIIDRC_BUTTON_ZL, "Left Z" },
	{ 8, WIIDRC_BUTTON_ZR, "Right Z" },
	{ 9, WIIDRC_BUTTON_A, "A" },
	{ 10, WIIDRC_BUTTON_B, "B" },
	{ 11, WIIDRC_BUTTON_X, "X" },
	{ 12, WIIDRC_BUTTON_Y, "Y" },
	{ 13, WIIDRC_BUTTON_PLUS, "+" },
	{ 14, WIIDRC_BUTTON_MINUS, "-" },
	{ 15, WIIDRC_BUTTON_HOME, "Home" },
	{ 16, WIIDRC_EXTRA_BUTTON_L3<<24, "LS-Button" },
	{ 17, WIIDRC_EXTRA_BUTTON_R3<<24, "RS-Button" },
	{ 18, R_STICK_U, "RS-Up" },
	{ 19, R_STICK_L, "RS-Left" },
	{ 20, R_STICK_R, "RS-Right" },
	{ 21, R_STICK_D, "RS-Down" },
	{ 22, L_STICK_U, "LS-Up" },
	{ 23, L_STICK_L, "LS-Left" },
	{ 24, L_STICK_R, "LS-Right" },
	{ 25, L_STICK_D, "LS-Down" },
};

static button_t analog_sources[] = {
		{ 0, L_STICK_AS_ANALOG, "Left Stick" },
		{ 1, R_STICK_AS_ANALOG, "Right Stick" },
};

static button_t menu_combos[] = {
		{ 0, WIIDRC_BUTTON_PLUS|WIIDRC_BUTTON_B, "Plus+B" },
		{ 1, WIIDRC_BUTTON_PLUS|WIIDRC_BUTTON_MINUS, "Plus+Minus" },
		{ 2, WIIDRC_BUTTON_HOME, "Home" },
		{ 3, WIIDRC_BUTTON_ZL|WIIDRC_BUTTON_ZR, "ZL+ZR" },
};

static void pause(int Control){ }

static void resume(int Control){ }

static void rumble(int Control, int rumble){ }

static void configure(int Control, controller_config_t* config){
	// Don't know how this should be integrated
}

static void assign(int p, int v){
	// TODO: Light up the LEDs appropriately
}

static int available(int Control){
	if(Control == 0)
		controller_WiiUGamepad.available[Control] = (WiiDRC_Inited() && WiiDRC_Connected()) ? 1 : 0;
	else
		controller_WiiUGamepad.available[Control] = 0;
	return controller_WiiUGamepad.available[Control];
}

static unsigned int getButtons(int Control)
{
	if (Control == 0 && WiiDRC_Inited() && WiiDRC_Connected())
	{
		const struct WiiDRCData *data = WiiDRC_Data();

		unsigned int btns = (unsigned int)data->button;

		if(data->xAxisL < -20) btns |= L_STICK_L;
		if(data->xAxisL >  20) btns |= L_STICK_R;
		if(data->yAxisL >  20) btns |= L_STICK_U;
		if(data->yAxisL < -20) btns |= L_STICK_D;

		if(data->xAxisR < -20) btns |= R_STICK_L;
		if(data->xAxisR >  20) btns |= R_STICK_R;
		if(data->yAxisR >  20) btns |= R_STICK_U;
		if(data->yAxisR < -20) btns |= R_STICK_D;

		// Slight hack: Add the extra buttons (L3/R3 basically) to the data.
		// They do not interfere with the other Wii U controller buttons.
		btns |= (data->extra)<<24;

		return btns;
	}

	return 0;
}

static int _GetKeys(int Control, BUTTONS * Keys, controller_config_t* config)
{
	if (wpadNeedScan){ WiiDRC_ScanPads(); wpadNeedScan = 0; }
	BUTTONS* c = Keys;
	memset(c, 0, sizeof(BUTTONS));
	//Reset buttons & sticks
	c->btns.All = 0xFFFF;
	c->leftStickX = c->leftStickY = c->rightStickX = c->rightStickY = 128;

	if (available(Control) == 0)
		return 0;

	unsigned int b = getButtons(Control);
	inline int isHeld(button_tp button){
		return (b & button->mask) == button->mask ? 0 : 1;
	}

	c->btns.SQUARE_BUTTON = isHeld(config->SQU);
	c->btns.CROSS_BUTTON = isHeld(config->CRO);
	c->btns.CIRCLE_BUTTON = isHeld(config->CIR);
	c->btns.TRIANGLE_BUTTON = isHeld(config->TRI);

	c->btns.R1_BUTTON = isHeld(config->R1);
	c->btns.L1_BUTTON = isHeld(config->L1);
	c->btns.R2_BUTTON = isHeld(config->R2);
	c->btns.L2_BUTTON = isHeld(config->L2);

	c->btns.L_DPAD = isHeld(config->DL);
	c->btns.R_DPAD = isHeld(config->DR);
	c->btns.U_DPAD = isHeld(config->DU);
	c->btns.D_DPAD = isHeld(config->DD);

	c->btns.START_BUTTON = isHeld(config->START);
	c->btns.R3_BUTTON = isHeld(config->R3);
	c->btns.L3_BUTTON = isHeld(config->L3);
	c->btns.SELECT_BUTTON = isHeld(config->SELECT);

	// When converting the stick coordinates to the PS1's range,
	// we need to invert the Y axes of both sticks, since it tends to
	// cause funny side-effects if we don't, like making Snake run down
	// instead of up in Metal Gear Solid, for example.
	c->leftStickX = convertToPSRange(WiiDRC_lStickX());
	c->leftStickY = convertToPSRange(-WiiDRC_lStickY());

	c->rightStickX = convertToPSRange(WiiDRC_rStickX());
	c->rightStickY = convertToPSRange(-WiiDRC_rStickY());

	// Return 1 if whether the exit button(s) are pressed
	return isHeld(config->exit) ? 0 : 1;
}

static void refreshAvailable(void);

controller_t controller_WiiUGamepad =
{ 
	'D', //for DRC I guess
	_GetKeys,
	configure,
	assign,
	pause,
	resume,
	rumble,
	refreshAvailable,
	{ 0, 0, 0, 0 },
	sizeof(buttons) / sizeof(buttons[0]),
	buttons,
	sizeof(analog_sources) / sizeof(analog_sources[0]),
	analog_sources,
	sizeof(menu_combos) / sizeof(menu_combos[0]),
	menu_combos,
	{ 
		.SQU = &buttons[12], // Y
		.CRO = &buttons[10], // B
		.CIR = &buttons[9],  // A
		.TRI = &buttons[11], // X
		.R1 = &buttons[6],  // Full R
		.L1 = &buttons[5],  // Full L
		.R2 = &buttons[8],  // Right Z
		.L2 = &buttons[7],  // Left Z
		.R3 = &buttons[17],  // Left Stick Button
		.L3 = &buttons[16],  // Right Stick Button
		.DL = &buttons[2],  // D-Pad Left
		.DR = &buttons[3],  // D-Pad Right
		.DU = &buttons[1],  // D-Pad Up
		.DD = &buttons[4],  // D-Pad Down
		.START = &buttons[13], // +
		.SELECT = &buttons[14], // -
		.analogL = &analog_sources[0], // Left Stick
		.analogR = &analog_sources[1], // Right stick
		.exit = &menu_combos[2], // Home
		.invertedYL = 0,
		.invertedYR = 0,
	}
};

static void refreshAvailable(void){
	int i = 0;

	for (i = 0; i < 4; ++i)
	{
		if(i == 0)
			controller_WiiUGamepad.available[i] = (WiiDRC_Inited() && WiiDRC_Connected()) ? 1 : 0;
		else
			controller_WiiUGamepad.available[i] = 0;
	}
}

#endif // HW_RVL
