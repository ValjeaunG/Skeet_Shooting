#pragma warning(disable:4996)
#include <iostream>
#include <time.h>

//SDL headers
#include "SDL2/include/SDL.h"
#include "SDL2/include/SDL_image.h"
#include "SDL2/include/SDL_mixer.h"

//load libraries
#pragma comment(lib,"SDL2\\lib\\x86\\SDL2.lib")
#pragma comment(lib,"SDL2\\lib\\x86\\SDL2main.lib")
#pragma comment(lib,"SDL2\\lib\\x86\\SDL2_image.lib")
#pragma comment(lib,"SDL2\\lib\\x86\\SDL2_mixer.lib")
#pragma comment(linker,"/subsystem:console")

//other headers
#include "Reticle.h"
#include "House.h"
#include "Target.h"

//functions
void draw_Text(SDL_Renderer* renderer, SDL_Texture* t, char* tex, int font_size, int dest_x, int dest_y)
{
	for (int i = 0; i < 16; i++)
	{
		//source
		SDL_Rect src;
		src.x = 64 * (tex[i] % 16);//column
		src.y = 64 * (tex[i] / 16);//row
		src.w = 64;
		src.h = 64;
		//destination
		SDL_Rect dest;
		dest.x = dest_x;
		dest.y = dest_y;
		dest.w = font_size;
		dest.h = font_size;

		//draw image
		//copy from source texture to destination screen.
		//SDL_FLIP_XXX enumeration allows you to mirror the image
		SDL_RenderCopyEx(renderer, t, &src, &dest, 0, NULL, SDL_FLIP_NONE);

		//increment dest_x!
		dest_x += dest.w;
	}
}

void draw_Text_Box(SDL_Renderer* renderer, int x, int y, int w, int h)
{
	//set area color to dark blue
	SDL_SetRenderDrawColor(renderer, 0, 0, 100, 150);

	SDL_Rect rect;
	rect.w = w;
	rect.h = h;
	rect.x = x;
	rect.y = y;

	SDL_RenderFillRect(renderer, &rect);
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
}

void s_Spawn_Target(Target* t, int window, int dir, int tier)
{
	if (window == 1 && dir == 1 || window == 2 && dir == 3)
	{//down right/left
		t->xvel;
		t->yvel;
	}
	else if (window == 1 && dir == 2 || window == 2 && dir == 4)
	{//up right/left
		t->xvel;
		t->yvel *= -1.f;
	}

	if (tier < 3) t->tier = 1;
	else if (tier >= 3 && tier < 6) t->tier = 2;
	else if (tier >= 6 && tier < 9) t->tier = 3;
	else if (tier >= 9) t->tier = 4;
	t->size = 16.f;
	t->timer = 0;
	t->hit = false;
	t->destroyed = false;
	t->is_offscreen = false;
	t->missed = false;
}

void t_Spawn_Target(Target* t, int window, int dir, int tier)
{
	switch (dir) {
	case 1: //up right
		t->xvel;
		t->yvel *= -1.f;
		break;
	case 2: //up far right
		t->xvel = 1.f;
		t->yvel *= -1.f;
		break;
	case 3: //up left
		t->xvel *= -1.f;
		t->yvel *= -1.f;
		break;
	case 4: //up far left
		t->xvel = -1.f;
		t->yvel *= -1.f;
		break;
	}

	if (tier < 3) t->tier = 1;
	else if (tier >= 3 && tier < 6) t->tier = 2;
	else if (tier >= 6 && tier < 9) t->tier = 3;
	else if (tier >= 9) t->tier = 4;
	t->size = 16.f;
	t->timer = 0;
	t->hit = false;
	t->destroyed = false;
	t->is_offscreen = false;
	t->missed = false;
}

void despawn_Targets(Target* t, int num)
{
	for (int i = 0; i < num; i++)
	{
		t[i].x = 0.f;
		t[i].y = 0.f;
		t[i].xvel = 0.f;
		t[i].yvel = 0.f;
		t[i].size = 0;
	}
}

void draw_Target(SDL_Renderer* renderer, Target* t)
{
	SDL_SetRenderDrawColor(renderer, t->r, t->g, t->b, 255);

	SDL_Rect rect;
	rect.w = t->size;
	rect.h = t->size;
	rect.x = t->x;
	rect.y = t->y;

	SDL_RenderFillRect(renderer, &rect);
}

int reticle_Overlap(Reticle* r, Target* t)
{
	float w = 0.5f * (r->size + t->size);
	float h = 0.5f * (r->size + t->size);
	float dx = r->x - t->x + 0.5f*(r->size - t->size);
	float dy = r->y - t->y + 0.5f*(r->size - t->size);

	if (dx*dx <= w * w && dy*dy <= h * h)
	{
		float wy = w * dy;
		float hx = h * dx;

		if (wy > hx) return (wy + hx > 0) ? 3 : 4;
		else return (wy + hx > 0) ? 2 : 1;
	}

	return 0;
}

int main(int argc, char** argv)
{
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

	//window properties
	int screen_w = 800;
	int screen_h = 600;
	SDL_Window* window = SDL_CreateWindow("Skeet", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screen_w, screen_h, SDL_WINDOW_SHOWN);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	//keyboard stuff
	unsigned char prev_key_state[256];
	unsigned char* keys = (unsigned char*)SDL_GetKeyboardState(NULL);
	const Uint8* state = SDL_GetKeyboardState(NULL);

	//frame rate stuff
	const int fps = 60;
	const int frame_delay = 1000 / fps;
	Uint32 frame_start;
	int frame_time;

	//sprite surfaces
	SDL_Surface* font_surface = IMG_Load("font_sheet.png");
	SDL_Surface* reticle_surface = IMG_Load("reticle.png");
	//sprite textures
	SDL_Texture* font_texture = SDL_CreateTextureFromSurface(renderer, font_surface);
	SDL_Texture* reticle_texture = SDL_CreateTextureFromSurface(renderer, reticle_surface);
	//free surfaces
	SDL_FreeSurface(font_surface);
	SDL_FreeSurface(reticle_surface);

	//sound setup
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);

	//load sound effects
	Mix_Chunk* shotgun_sfx = Mix_LoadWAV("shotgun-old_school.wav");
	Mix_Chunk* shotgun2_sfx = Mix_LoadWAV("shotgun-spas12.wav");
	Mix_Chunk* pump_sfx = Mix_LoadWAV("pump_shotgun.wav");
	Mix_Chunk* reload_sfx = Mix_LoadWAV("shotgun-reload.wav");
	Mix_Chunk* launcher_sfx = Mix_LoadWAV("bird-man_gun-shot.wav");
	Mix_Chunk* hit_sfx = Mix_LoadWAV("impact-ringy_sfx.wav");
	Mix_Chunk* break_sfx = Mix_LoadWAV("impact-hard_sfx.wav");

	//reticle stuff
	int reticle_sz = 32;
	Reticle reticle;
	reticle.init_Reticle(reticle_sz);
	reticle.init_Source_Img(0, 0, 32, 32, 2);
	reticle.frame_update = SDL_GetTicks();

	//house stuff
	int which_window = 0;
	House high_house;
	House low_house;
	House trap_house;
	high_house.init_House(0, 200, 100, 400);
	high_house.init_Window(10, 210, 80, 80);
	low_house.init_House(700, 400, 100, 200);
	low_house.init_Window(710, 410, 80, 80);
	trap_house.init_House(250, 500, 350, 100);
	trap_house.init_Window(335, 510, 80, 80);
	trap_house.init_Window2(430, 510, 80, 80);

	//target stuff
	int max_boxes = 101;
	int active_boxes = 0;
	int destroyed_boxes = 0;
	int missed_boxes = 0;
	int hit_time = 0;
	int dir_change = 0;
	int spawn_timer = 300;
	int timer_change = 0;
	int tier_change = 1;
	int tier_change_t = 7000;
	Target* box = (Target*)malloc(sizeof(Target)*max_boxes);
	
	//shotgun stuff
	int ammo = 0;
	int max_ammo = 0;
	int shot_delay = 0;
	int fired_shots = 0;
	int reload_delay = 0;
	int reload_pump_sfx_t = 0;
	int damage = 0;
	int m_damage_chance = 0;
	int recoil_time = 0;
	int m_recoil_time = 0;
	float recoil_amount = 0.f;
	int display_controls_t = 0;

	//shotgun flags
	bool has_shotgun = false;
	bool double_barrel = false;
	bool pump_action = false;
	bool has_ammo = false;
	bool gauge20 = false;
	bool gauge12 = false;
	bool gauge10 = false;
	bool shot_fired = false;
	bool reloading = false;
	bool pumped = true;

	//other flags
	bool running = true;
	bool game_started = false;
	bool paused = false;
	bool game_finished = false;
	bool selection_stage = false;
	bool skeet_mode = true;
	bool trap_mode = false;
	bool t = false;
	bool r = false;
	bool a = false;
	bool p = false;

	//text stuff
	char title_text[17];
	char start_text[17];
	char quit_text[17];
	char mode_text[17];
	char select_text[17];
	char barrel_text[17];
	char barrel_text2[17];
	char barrel_text3[17];
	char pump_text[17];
	char pump_text2[17];
	char pump_text3[17];
	char gauge20_text[17];
	char gauge20_text2[17];
	char gauge20_text3[17];
	char gauge20_text4[17];
	char gauge12_text[17];
	char gauge12_text2[17];
	char gauge12_text3[17];
	char gauge12_text4[17];
	char gauge10_text[17];
	char gauge10_text2[17];
	char gauge10_text3[17];
	char gauge10_text4[17];
	char selection_complete[17];
	char begin_text[17];
	char gback_text[17];
	char score_text[17];
	char multiplier_text[17];
	char chain_text[17];
	char pause_text[17];
	char paused_menu[17];
	char resume_text[17];
	char retry_text[17];
	char back_text[17];
	char controls_text[17];
	char controls2_text[17];
	char ammo_text[17];
	char reload_text[17];
	char results_text[17];
	char d_boxes_text[17];
	char b_missed_text[17];
	char s_missed_text[17];
	char bonus_text[17];
	char total_text[17];
	sprintf(start_text, "S-Start");
	sprintf(quit_text, "Q-Quit");
	sprintf(select_text, "Shotgun & Ammo");
	sprintf(barrel_text, "1-Double Barrel");
	sprintf(barrel_text2, "-Quickshot");
	sprintf(barrel_text3, "-Only 2 Ammo");
	sprintf(pump_text, "2-Pump Action");
	sprintf(pump_text2, "-Slow Fire Rate");
	sprintf(pump_text3, "-8 Ammo");
	sprintf(gauge20_text, "3-20 Gauge");
	sprintf(gauge20_text2, "-Low Damage");
	sprintf(gauge20_text3, "-Low Recoil");
	sprintf(gauge20_text4, "-Fast Reload");
	sprintf(gauge12_text, "4-12 Gauge");
	sprintf(gauge12_text2, "-High Damage");
	sprintf(gauge12_text3, "-Average Recoil");
	sprintf(gauge12_text4, "-Average Reload");
	sprintf(gauge10_text, "5-10 Gauge");
	sprintf(gauge10_text2, "-Very Powerful");
	sprintf(gauge10_text3, "-High Recoil");
	sprintf(gauge10_text4, "-Slow Reload");
	sprintf(selection_complete, "Gun Acquired");
	sprintf(begin_text, "B-Begin");
	sprintf(gback_text, "G-Go Back");
	sprintf(pause_text, "P-Pause");
	sprintf(paused_menu, "Paused");
	sprintf(resume_text, "E-Resume");
	sprintf(retry_text, "S-Start Over");
	sprintf(back_text, "B-Back to title");
	sprintf(controls_text, "W,A,S,D-Move");
	sprintf(controls2_text, "Space-Shoot");
	sprintf(reload_text, "R-Reload");
	sprintf(results_text, "Results");

	//misc
	int score = 0;
	int multiplier = 1;
	int chain = 0;
	int bonus_points = 1000;
	int total_score = 0;
	int end_timer = 0;
	srand(time(0));

	while (running)
	{
		//copies contents of keys to prev_key_state
		memcpy(prev_key_state, keys, 256);

		//gets milliseconds at the start of a frame
		frame_start = SDL_GetTicks();

		//consume all window events first
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{ if (event.type == SDL_QUIT) exit(0); }

		//set window screen color to black
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
		//clear screen
		SDL_RenderClear(renderer);

		//start and quit
		if (state[SDL_SCANCODE_S] && !game_started) selection_stage = true;
		if (state[SDL_SCANCODE_Q] && !selection_stage && !game_started) running = false;

		//game modes
		{
			if (!selection_stage && !game_started)
			{
				if (skeet_mode)
				{
					sprintf(title_text, "Skeet");
					sprintf(mode_text, "T,R,A,P");
					if (state[SDL_SCANCODE_T]) t = true;
					if (state[SDL_SCANCODE_R]) r = true;
					if (state[SDL_SCANCODE_A]) a = true;
					if (state[SDL_SCANCODE_P]) p = true;
					if (t && r && a && p)
					{
						trap_mode = true;
						skeet_mode = false;
					}
				}
				if (trap_mode)
				{
					sprintf(title_text, "Trap");
					sprintf(mode_text, "1-Skeet");
					if (state[SDL_SCANCODE_1])
					{
						skeet_mode = true;
						t = false;
						r = false;
						a = false;
						p = false;
						trap_mode = false;
					}
				}
			}
		}

		//selection menu
		if (selection_stage)
		{
			//picking shotgun
			{
				if (!has_shotgun && state[SDL_SCANCODE_1])
				{
					has_shotgun = true;
					double_barrel = true;
					ammo = 2;
					max_ammo = 2;
					shot_delay = 5;
				}
				else if (!has_shotgun && state[SDL_SCANCODE_2])
				{
					has_shotgun = true;
					pump_action = true;
					ammo = 8;
					max_ammo = 8;
					shot_delay = 20;
				}
			}
			
			//picking ammo
			{
				if (has_shotgun && !has_ammo && state[SDL_SCANCODE_3])
				{
					has_ammo = true;
					gauge20 = true;
					gauge12 = false;
					gauge10 = false;
					recoil_amount = 2.f;
					m_recoil_time = 13;
					reload_delay = 10;
				}
				else if (has_shotgun && !has_ammo && state[SDL_SCANCODE_4])
				{
					has_ammo = true;
					gauge20 = false;
					gauge12 = true;
					gauge10 = false;
					recoil_amount = 5.5f;
					m_recoil_time = 18;
					reload_delay = 25;
				}
				else if (has_shotgun && !has_ammo && state[SDL_SCANCODE_5])
				{
					has_ammo = true;
					gauge20 = false;
					gauge12 = false;
					gauge10 = true;
					recoil_amount = 9.f;
					m_recoil_time = 21;
					reload_delay = 50;
				}
			}
			
			//start game or go back to title screen
			if (has_shotgun && has_ammo)
			{
				if (state[SDL_SCANCODE_B])
				{
					game_started = true;
					selection_stage = false;
					score = 0;
					total_score = 0;
					multiplier = 1;
					chain = 0;
					bonus_points = 1000;
					fired_shots = 0;
					active_boxes = 0;
					destroyed_boxes = 0;
					missed_boxes = 0;
					tier_change_t = 7000;
				}
				if (state[SDL_SCANCODE_G])
				{
					has_shotgun = false;
					double_barrel = false;
					pump_action = false;
					has_ammo = false;
					gauge20 = false;
					gauge12 = false;
					gauge10 = false;
					selection_stage = false;
				}
			}
		}

		//pause and resume
		if (state[SDL_SCANCODE_P] && game_started && !game_finished) paused = true;
		else if (state[SDL_SCANCODE_E]) paused = false;

		//start over
		if (paused && state[SDL_SCANCODE_S] || game_finished && state[SDL_SCANCODE_S])
		{
			score = 0;
			total_score = 0;
			multiplier = 1;
			chain = 0;
			bonus_points = 1000;
			fired_shots = 0;
			active_boxes = 0;
			destroyed_boxes = 0;
			missed_boxes = 0;
			tier_change_t = 7000;
			end_timer = 0;
			if (double_barrel) ammo = 2;
			if (pump_action) ammo = 8;
			despawn_Targets(box, max_boxes);
			reticle.init_Reticle(reticle_sz);
			paused = false;
			shot_fired = false;
			game_finished = false;
		}

		//back to title screen
		if (paused && state[SDL_SCANCODE_B] || game_finished && state[SDL_SCANCODE_B])
		{
			game_started = false;
			game_finished = false;
			paused = false;
			has_shotgun = false;
			double_barrel = false;
			pump_action = false;
			has_ammo = false;
			shot_fired = false;
			despawn_Targets(box, max_boxes);
			reticle.init_Reticle(reticle_sz);
			display_controls_t = 0;
			end_timer = 0;
		}

		//game stats
		{
			sprintf(ammo_text, "Ammo:%d", ammo);
			sprintf(score_text, "Score:%d", score);
			sprintf(multiplier_text, "Multiplier:%d", multiplier);
			sprintf(chain_text, "Chain:%d", chain);
			sprintf(d_boxes_text, "Destroyed:%d", destroyed_boxes);
			sprintf(b_missed_text, "Missed:%d", missed_boxes);
			sprintf(s_missed_text, "Shots:%d", fired_shots);
			sprintf(bonus_text, "Bonus:%d", bonus_points);
			sprintf(total_text, "Total:%d", total_score);
		}

		//gameplay
		if (game_started && !game_finished && !paused)
		{
			//shotgun mechanics
			{
				if (display_controls_t < 300) display_controls_t++;
				reticle.move_Controls(state[SDL_SCANCODE_A], state[SDL_SCANCODE_D], state[SDL_SCANCODE_W], state[SDL_SCANCODE_S], 0, screen_w, 0, screen_h);

				unsigned int current_time = SDL_GetTicks();
				reticle.animate_Func(current_time);

				//shooting and recoil
				{
					if (double_barrel || pump_action && pumped)
					{ if (state[SDL_SCANCODE_SPACE] && !state[SDL_SCANCODE_R]) shot_fired = true; }
					if (shot_fired && recoil_time != 25 && ammo > 0)
					{
						if (recoil_time < 25) recoil_time++;
						if (recoil_time == 1) fired_shots += 1;
						if (recoil_time < m_recoil_time) reticle.y -= recoil_amount;
						else if (recoil_time >= m_recoil_time) reticle.y += recoil_amount;

						//damage profile
						{
							if (gauge20)
							{
								m_damage_chance = rand() % 4 + 1;
								if (m_damage_chance < 4) damage = 80;
								else if (m_damage_chance == 4) damage = 69;
							}
							if (gauge12)
							{
								m_damage_chance = rand() % 8 + 1;
								if (m_damage_chance > 1) damage = 110;
								else if (m_damage_chance == 1) damage = 99;
							}
							if (gauge10) damage = 200;
						}

						//play shotgun sfx
						{
							if (recoil_time == 1)
							{
								if (double_barrel || pump_action && pumped)
								{
									if (gauge20 || gauge12) Mix_PlayChannel(-1, shotgun_sfx, 0);
									else if (gauge10) Mix_PlayChannel(-1, shotgun2_sfx, 0);

									pumped = false;
								}
							}
							if (recoil_time == 20)
							{
								if (pump_action && ammo != 0)
								{
									pumped = true;
									Mix_PlayChannel(-1, pump_sfx, 0);
								}
							}
						}
					}
				}

				//rate of fire
				{
					if (recoil_time == 25)
					{
						if (shot_delay > 0) shot_delay--;
						if (shot_delay == 0)
						{
							ammo -= 1;
							if (double_barrel) shot_delay = 5;
							if (pump_action && pumped) shot_delay = 20;
							recoil_time = 0;
							shot_fired = false;
						}
					}
				}

				//reload
				{
					if (ammo >= 0 && ammo < max_ammo && state[SDL_SCANCODE_R]) reloading = true;
					if (reloading)
					{
						if (reload_delay > 0) reload_delay--;
						if (reload_delay == 0)
						{
							ammo += 1;
							if (gauge20) reload_delay = 10;
							if (gauge12) reload_delay = 25;
							if (gauge10) reload_delay = 50;
							reloading = false;
							shot_fired = false;
							pumped = false;
							Mix_PlayChannel(-1, reload_sfx, 0);
							reload_pump_sfx_t = 0;
						}
					}
					if (pump_action && !pumped && !state[SDL_SCANCODE_R])
					{
						if (state[SDL_SCANCODE_SPACE]) reload_pump_sfx_t = 0;
						else if (!state[SDL_SCANCODE_SPACE]) reload_pump_sfx_t++;
						if (reload_pump_sfx_t == 5)
						{
							pumped = true;
							Mix_PlayChannel(-1, pump_sfx, 0);
						}
					}
				}

				//multiplier and bonus increase
				{
					switch (chain) {
					case 10:
						multiplier = 2;
						if (recoil_time == 1) bonus_points += 1000;
						break;
					case 20:
						multiplier = 3;
						if (recoil_time == 1) bonus_points += 5000;
						break;
					case 40:
						multiplier = 4;
						if (recoil_time == 1) bonus_points += 20000;
						break;
					case 80:
						multiplier = 5;
						if (recoil_time == 1) bonus_points += 40000;
						break;
					}

					if (destroyed_boxes > 49 && destroyed_boxes < 75 && end_timer == 1) bonus_points *= 2;
					else if (destroyed_boxes > 74 && destroyed_boxes < 99 && end_timer == 1) bonus_points *= 3;
					else if (destroyed_boxes >= 99 && end_timer == 1) bonus_points *= 4;
				}
			}
			
			{} ///dont mess with this

			//flying boxes
			{
				if (spawn_timer > 0 && active_boxes < max_boxes) spawn_timer--;

				if (tier_change_t > 0)
				{
					tier_change_t--;
					tier_change = 1;
				}

				if (bonus_points < 0) bonus_points = 0;

				for (int i = 0; i < max_boxes; i++)
				{
					//spawn and movement
					{
						if (spawn_timer == 0)
						{
							Mix_PlayChannel(-1, launcher_sfx, 0);
							active_boxes += 1;
							if (tier_change_t == 0) tier_change = rand() % 10 + 1;
							dir_change = rand() % 4 + 1;
							timer_change = rand() % 4 + 1;
							switch (timer_change) {
							case 1:
								if (tier_change_t > 0) spawn_timer = 300;
								else if (tier_change_t == 0) spawn_timer = 150;
								else if (tier_change_t == 0 && !gauge20 && destroyed_boxes > 74) spawn_timer = 100;
								break;
							case 2:
								if (tier_change_t > 0) spawn_timer = 100;
								else if (tier_change_t == 0) spawn_timer = 50;
								else if (tier_change_t == 0 && !gauge20 && destroyed_boxes > 74) spawn_timer = 33;
								break;
							case 3:
								if (tier_change_t > 0) spawn_timer = 500;
								else if (tier_change_t == 0) spawn_timer = 250;
								else if (tier_change_t == 0 && !gauge20 && destroyed_boxes > 74) spawn_timer = 167;
								break;
							case 4:
								spawn_timer = 300;
								if (tier_change_t == 0 && !gauge20 && destroyed_boxes > 74) spawn_timer = 200;
								break;
							}

							which_window = rand() % 2 + 1;

							if (skeet_mode)
							{
								if (which_window == 1)
								{
									box[active_boxes].x = high_house.xwindow + 40;
									box[active_boxes].y = high_house.ywindow + 40;
									box[active_boxes].xvel = 1.5f;
								}
								else if (which_window == 2)
								{
									box[active_boxes].x = low_house.xwindow + 40;
									box[active_boxes].y = low_house.ywindow + 40;
									box[active_boxes].xvel = -1.5f;
								}

								box[active_boxes].yvel = 0.25f;

								s_Spawn_Target(&box[active_boxes], which_window, dir_change, tier_change);
							}

							if (trap_mode)
							{
								if (which_window == 1)
								{
									box[active_boxes].x = trap_house.xwindow + 40;
									box[active_boxes].y = trap_house.ywindow + 40;
								}
								else if (which_window == 2)
								{
									box[active_boxes].x = trap_house.xwindow2 + 40;
									box[active_boxes].y = trap_house.ywindow2 + 40;
								}

								box[active_boxes].xvel = 0.25f;
								box[active_boxes].yvel = 1.5f;

								t_Spawn_Target(&box[active_boxes], which_window, dir_change, tier_change);
							}
						}
						
						if (display_controls_t == 300)
						{
							if (!box[i].destroyed && !box[i].is_offscreen)
							{
								box[i].x += box[i].xvel;
								box[i].y += box[i].yvel;
							}
						}
						
						//missed targets via offscreen
						{
							if (box[i].x < 0 || box[i].x > screen_w || box[i].y < 0 || box[i].y > screen_h)
							{
								if (!box[i].missed) box[i].is_offscreen = true;
							}
							if (box[i].is_offscreen)
							{
								if (box[i].timer < 50) box[i].timer++;
								else if (box[i].timer == 50)
								{
									chain = 0;
									multiplier = 1;
									bonus_points -= 2500;
									missed_boxes += 1;
									box[i].timer = 0;
									box[i].missed = true;
								}
							}

							if (box[i].missed) box[i].is_offscreen = false;
						}
					}

					{}

					//box tiers
					{
						switch (box[i].tier) {
						case 1:
							box[i].r = 0;
							box[i].g = 255;
							box[i].b = 0;
							box[i].health = 50;
							break;
						case 2:
							box[i].r = 255;
							box[i].g = 255;
							box[i].b = 0;
							box[i].health = 75;
							break;
						case 3:
							box[i].r = 255;
							box[i].g = 0;
							box[i].b = 0;
							box[i].health = 100;
							break;
						case 4:
							box[i].r = 255;
							box[i].g = 0;
							box[i].b = 255;
							box[i].health = 150;
							break;
						}
					}

					//target health
					{
						int overlap = reticle_Overlap(&reticle, &box[i]);
						
						if (!box[i].destroyed)
						{
							if (overlap != 0 && recoil_time == 1)
							{
								box[i].hit = true;
								hit_time++;
								if (hit_time == 1)
								{
									box[i].health -= damage;
									if (box[i].health <= 0)
									{
										box[i].destroyed = true;
										score += 100 * multiplier;
										chain += 1;
										destroyed_boxes += 1;
										Mix_PlayChannel(-1, break_sfx, 0);
									}
									else if (box[i].health > 0) 
									{ 
										score += 50; 
										Mix_PlayChannel(-1, hit_sfx, 0);
									}
									hit_time = 0;
								}
							}

							if (box[i].hit && !box[i].destroyed)
							{
								if (box[i].health > 75) box[i].tier = 3;
								else if (box[i].health > 50) box[i].tier = 2;
								else if (box[i].health < 50 && box[i].health > 0) box[i].tier = 1;
							}
						}
					}
				}
			}

			{}

			//end game
			{
				total_score = score + bonus_points;
				if (active_boxes == max_boxes) end_timer++;
				if (end_timer > 700) game_finished = true;
			}
		}
		
		{}

		//draw
		{
			//title screen text
			if (!game_started && !selection_stage)
			{
				draw_Text(renderer, font_texture, title_text, 50, 290, 10);
				draw_Text(renderer, font_texture, start_text, 25, 315, 275);
				draw_Text(renderer, font_texture, quit_text, 25, 315, 500);
				draw_Text(renderer, font_texture, mode_text, 10, 350, 575);
			}

			//selection menu text
			if (!game_started && selection_stage)
			{
				if (!has_shotgun || !has_ammo) draw_Text(renderer, font_texture, select_text, 25, 225, 50);

				if (!has_shotgun)
				{
					draw_Text(renderer, font_texture, barrel_text, 25, 225, 150);
					draw_Text(renderer, font_texture, barrel_text2, 15, 225, 175);
					draw_Text(renderer, font_texture, barrel_text3, 15, 225, 200);
					draw_Text(renderer, font_texture, pump_text, 25, 225, 250);
					draw_Text(renderer, font_texture, pump_text2, 15, 225, 275);
					draw_Text(renderer, font_texture, pump_text3, 15, 225, 300);
				}
				else if (has_shotgun && !has_ammo)
				{
					draw_Text(renderer, font_texture, gauge20_text, 25, 275, 150);
					draw_Text(renderer, font_texture, gauge20_text2, 15, 275, 175);
					draw_Text(renderer, font_texture, gauge20_text3, 15, 275, 200);
					draw_Text(renderer, font_texture, gauge20_text4, 15, 275, 225);
					draw_Text(renderer, font_texture, gauge12_text, 25, 275, 275);
					draw_Text(renderer, font_texture, gauge12_text2, 15, 275, 300);
					draw_Text(renderer, font_texture, gauge12_text3, 15, 275, 325);
					draw_Text(renderer, font_texture, gauge12_text4, 15, 275, 350);
					draw_Text(renderer, font_texture, gauge10_text, 25, 275, 400);
					draw_Text(renderer, font_texture, gauge10_text2, 15, 275, 425);
					draw_Text(renderer, font_texture, gauge10_text3, 15, 275, 450);
					draw_Text(renderer, font_texture, gauge10_text4, 15, 275, 475);
				}
				else if (has_shotgun && has_ammo)
				{
					draw_Text(renderer, font_texture, selection_complete, 25, 250, 50);
					draw_Text(renderer, font_texture, begin_text, 25, 315, 200);
					draw_Text(renderer, font_texture, gback_text, 25, 295, 500);
				}
			}

			if (game_started && !game_finished)
			{
				//houses
				{
					if (skeet_mode)
					{
						high_house.draw_House(renderer);
						high_house.draw_Window(renderer);
						low_house.draw_House(renderer);
						low_house.draw_Window(renderer);
					}
					if (trap_mode)
					{
						trap_house.draw_House(renderer);
						trap_house.draw_Window(renderer);
						trap_house.draw_Window2(renderer);
					}
				}

				for (int i = 0; i < max_boxes; i++)
				{ if (!box[i].destroyed) draw_Target(renderer, &box[i]); }

				//reticle.draw_Reticle_Hitbox(renderer);
				reticle.draw_Reticle_Img(renderer, reticle_texture, reticle.img_source_x, reticle.img_source_y, reticle.img_source_w, reticle.img_source_h);

				//player controls text
				if (display_controls_t < 300)
				{
					draw_Text(renderer, font_texture, controls_text, 25, 10, 100);
					draw_Text(renderer, font_texture, controls2_text, 25, 500, 100);
				}

				//pause menu text
				if (!paused) draw_Text(renderer, font_texture, pause_text, 15, 650, 10);
				else if (paused)
				{
					draw_Text_Box(renderer, 225, 125, 400, 200);
					draw_Text(renderer, font_texture, paused_menu, 50, 275, 125);
					draw_Text(renderer, font_texture, resume_text, 25, 325, 200);
					draw_Text(renderer, font_texture, retry_text, 25, 275, 250);
					draw_Text(renderer, font_texture, back_text, 25, 225, 300);
				}

				//HUD text
				{
					draw_Text_Box(renderer, 5, 550, 150, 100);
					draw_Text(renderer, font_texture, score_text, 15, 10, 10);
					draw_Text(renderer, font_texture, multiplier_text, 15, 225, 10);
					draw_Text(renderer, font_texture, chain_text, 15, 450, 10);
					draw_Text(renderer, font_texture, ammo_text, 25, 10, 550);
					draw_Text(renderer, font_texture, reload_text, 15, 10, 575);
				}
			}

			if (game_finished)
			{
				draw_Text(renderer, font_texture, results_text, 50, 225, 10);
				draw_Text(renderer, font_texture, score_text, 25, 200, 150);
				draw_Text(renderer, font_texture, d_boxes_text, 25, 200, 200);
				draw_Text(renderer, font_texture, b_missed_text, 25, 200, 250);
				draw_Text(renderer, font_texture, s_missed_text, 25, 200, 300);
				draw_Text(renderer, font_texture, bonus_text, 25, 200, 350);
				draw_Text(renderer, font_texture, total_text, 25, 200, 400);
				draw_Text(renderer, font_texture, retry_text, 25, 275, 500);
				draw_Text(renderer, font_texture, back_text, 25, 225, 550);
			}
		}

		//updates screen
		SDL_RenderPresent(renderer);

		//caps frame rate
		frame_time = SDL_GetTicks() - frame_start;
		if (frame_delay > frame_time) SDL_Delay(frame_delay - frame_time);
	}

	return 0;
}