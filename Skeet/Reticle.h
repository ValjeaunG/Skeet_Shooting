#pragma once

struct Reticle
{
	int size, img_source_x, img_source_y, img_source_w, img_source_h, sprite_sheet;
	unsigned int frame_update;
	float x, y, xvel, yvel;

	void init_Reticle(int sz)
	{
		size = sz;
		x = 400;
		y = 300;
		xvel = 0;
		yvel = 0;
	}

	void move_Controls(const Uint8 input1, const Uint8 input2, const Uint8 input3, const Uint8 input4, int bound1, int bound2, int bound3, int bound4)
	{
		//if not at edges of screen
		if (input1) xvel = -7.f;
		else if (input2) xvel = 7.f;
		if (input3) yvel = -7.f;
		else if (input4) yvel = 7.f;

		if (input1 || input2) x += xvel;
		if (input3 || input4) y += yvel;

		//if at edges of screen
		if (x <= bound1) x = 0;
		else if (x >= bound2 - size) x = bound2 - size;
		if (y <= bound3) y = 0;
		else if (y >= bound4 - size) y = bound4 - size;
	}

	void draw_Reticle_Hitbox(SDL_Renderer* renderer)
	{
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

		SDL_Rect rect;
		rect.w = size;
		rect.h = size;
		rect.x = x;
		rect.y = y;

		SDL_RenderFillRect(renderer, &rect);
	}

	void init_Source_Img(int x_src, int y_src, int w_src, int h_src, int n_frames)
	{
		img_source_x = x_src;
		img_source_y = y_src;
		img_source_w = w_src;
		img_source_h = h_src;
		sprite_sheet = img_source_h * n_frames;
	}

	void draw_Reticle_Img(SDL_Renderer *renderer, SDL_Texture *t, int x_src, int y_src, int w_src, int h_src)
	{
		//source
		SDL_Rect src;
		src.x = x_src;
		src.y = y_src;
		src.w = w_src;
		src.h = h_src;
		//destination
		SDL_Rect dest;
		dest.x = x;
		dest.y = y;
		dest.w = src.w;
		dest.h = src.h;
		//draw image
		//copy from source texture to destination screen.
		SDL_RenderCopyEx(renderer, t, &src, &dest, 0, NULL, SDL_FLIP_NONE);
	}

	void animate_Func(unsigned int time)
	{
		if (time - frame_update > 100)
		{
			frame_update = time;

			//update
			img_source_y += img_source_h;
		}
		//reset
		if (img_source_y >= sprite_sheet) img_source_y = 0;
	}
};
