#pragma once

struct House
{
	int x, y, w, h;
	int  xwindow, ywindow, w_window, h_window, xwindow2, ywindow2, w_window2, h_window2;

	void init_House(int hx, int hy, int hw, int hh)
	{
		x = hx;
		y = hy;
		w = hw;
		h = hh;
	}

	void init_Window(int x, int y, int w, int h)
	{
		xwindow = x;
		ywindow = y;
		w_window = w;
		h_window = h;
	}

	void init_Window2(int x, int y, int w, int h)
	{
		xwindow2 = x;
		ywindow2 = y;
		w_window2 = w;
		h_window2 = h;
	}

	void draw_House(SDL_Renderer* renderer)
	{
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

		SDL_Rect rect;
		rect.w = w;
		rect.h = h;
		rect.x = x;
		rect.y = y;

		SDL_RenderFillRect(renderer, &rect);
	}

	void draw_Window(SDL_Renderer* renderer)
	{
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

		SDL_Rect rect;
		rect.w = w_window;
		rect.h = h_window;
		rect.x = xwindow;
		rect.y = ywindow;

		SDL_RenderFillRect(renderer, &rect);
	}

	void draw_Window2(SDL_Renderer* renderer)
	{
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

		SDL_Rect rect;
		rect.w = w_window2;
		rect.h = h_window2;
		rect.x = xwindow2;
		rect.y = ywindow2;

		SDL_RenderFillRect(renderer, &rect);
	}
};
