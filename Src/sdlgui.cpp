/*
 * sdlgui.cpp 
 *
 * This file is taken from the ARAnyM project which builds a new and powerful
 * TOS/FreeMiNT compatible virtual machine running on almost any hardware.
 *
 * Copyright (c) 2001 Thomas Huth - taken from his hatari project
 * Copyright (c) 2002-2005 Petr Stehlik of ARAnyM dev team (see AUTHORS)
 *
 * It is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * You should have received a copy of the GNU General Public License
 * along with Frodo; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "sysdeps.h"
#include "sdlgui.h"

#include <cstdlib>

#include "font8.h"

static SDL_Surface *mainsurface = NULL;
#define sdlscrn mainsurface

static SDL_Surface *fontgfx=NULL;
static int fontwidth, fontheight;   /* Height and width of the actual font */

// Stores current dialog coordinates
static SDL_Rect DialogRect = {0, 0, 0, 0};

// Used by SDLGui_Get[First|Next]BackgroundRect()
static SDL_Rect BackgroundRect = {0, 0, 0, 0};
static int BackgroundRectCounter;
enum
{
  SG_BCKGND_RECT_BEGIN,
  SG_BCKGND_RECT_TOP,
  SG_BCKGND_RECT_LEFT,
  SG_BCKGND_RECT_RIGHT,
  SG_BCKGND_RECT_BOTTOM,
  SG_BCKGND_RECT_END
};

SDL_Color blackc[]     = {{0, 0, 0, 0}};
SDL_Color darkgreyc[]  = {{128, 128, 128, 0}};
SDL_Color greyc[]      = {{192, 192, 192, 0}};
SDL_Color whitec[]     = {{255, 255, 255, 0}};

enum
{
  SG_FIRST_EDITFIELD,
  SG_PREVIOUS_EDITFIELD,
  SG_NEXT_EDITFIELD,
  SG_LAST_EDITFIELD
};

/*-----------------------------------------------------------------------*/
/*
  Load an 1 plane XBM into a 8 planes SDL_Surface.
*/
static SDL_Surface *SDLGui_LoadXBM(int w, int h, Uint8 *srcbits)
{
  SDL_Surface *bitmap;
  Uint8 *dstbits;
  int x, y, srcpitch;

  /* Allocate the bitmap */
  bitmap = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 8, 0, 0, 0, 0);
  if ( bitmap == NULL )
  {
  // TODO  panicbug("Couldn't allocate bitmap: %s", SDL_GetError());
    return(NULL);
  }

  srcpitch = ((w + 7) / 8);
  dstbits = (Uint8 *)bitmap->pixels;
  int mask = 1;

  /* Copy the pixels */
  for (y = 0 ; y < h ; y++)
  {
    for (x = 0 ; x < w ; x++)
    {
      dstbits[x] = (srcbits[x / 8] & mask) ? 1 : 0;
      mask <<= 1;
      mask |= (mask >> 8);
      mask &= 0xFF;
    }
    dstbits += bitmap->pitch;
    srcbits += srcpitch;
  }

  return(bitmap);
}

/*-----------------------------------------------------------------------*/
/*
  Initialize the GUI.
*/
bool SDLGui_Init(SDL_Surface *GUISurface)
{
  mainsurface = GUISurface;
  /* Load the font graphics: */
  fontgfx = SDLGui_LoadXBM(font8_width, font8_height, font8_bits);
  if (fontgfx == NULL)
  {
// TODO    panicbug("Could not create font data");
// TODO    panicbug("ARAnyM GUI will not be available");
    return false;
  }

  /* Set font color 0 as transparent */
  SDL_SetColorKey(fontgfx, SDL_SRCCOLORKEY, 0);

  /* Get the font width and height: */
  fontwidth = fontgfx->w/16;
  fontheight = fontgfx->h/16;

  return true;
}


/*-----------------------------------------------------------------------*/
/*
  Uninitialize the GUI.
*/
int SDLGui_UnInit()
{
  if (fontgfx)
  {
    SDL_FreeSurface(fontgfx);
    fontgfx = NULL;
  }

  return 0;
}


/*-----------------------------------------------------------------------*/
/*
  Compute real coordinates for a given object.
  Note: centers dialog on screen.
*/
static void SDLGui_ObjCoord(SGOBJ *dlg, int objnum, SDL_Rect *rect)
{
  rect->x = dlg[objnum].x * fontwidth;
  rect->y = dlg[objnum].y * fontheight;
  rect->w = dlg[objnum].w * fontwidth;
  rect->h = dlg[objnum].h * fontheight;

  rect->x += (sdlscrn->w - (dlg[0].w * fontwidth)) / 2;
  rect->y += (sdlscrn->h - (dlg[0].h * fontheight)) / 2;
}


/*-----------------------------------------------------------------------*/
/*
  Compute real coordinates for a given object.
  This one takes borders into account and give coordinates as seen by user
*/
void SDLGui_ObjFullCoord(SGOBJ *dlg, int objnum, SDL_Rect *coord)
{
  SDLGui_ObjCoord(dlg, objnum, coord);

  switch (dlg[objnum].type)
  {
    case SGBOX:
    case SGBUTTON:
      {
        // Take border into account
        int border_size;

        if (dlg[objnum].flags & SG_SELECTABLE)
        {
          if (dlg[objnum].flags & SG_DEFAULT)
            border_size = 4;
          else
            border_size = 3;
        }
        else
        {
          if (dlg[objnum].flags & SG_BACKGROUND)
            border_size = 6;
          else
            border_size = 5;
        }

        coord->x -= border_size;
        coord->y -= border_size;
        coord->w += (border_size * 2);
        coord->h += (border_size * 2);
      }
      break;
    case SGEDITFIELD:
      // Allow one more pixel to the right for cursor
      coord->w += 1;
      // There is a line below
      coord->h += 1;
      break;
  }
}


/*-----------------------------------------------------------------------*/
/*
  Refresh display at given coordinates.
  Unlike SDL_UpdateRect() this function can eat coords that goes beyond screen
  boundaries.
  "rect" will be modified to represent the area actually refreshed.
*/
void SDLGui_UpdateRect(SDL_Rect *rect)
{
  if (rect->x < 0)
  {
    rect->w += rect->x;
    rect->x = 0;
  }
  if ((rect->x + rect->w) > sdlscrn->w)
    rect->w = (sdlscrn->w - rect->x);

  if (rect->y < 0)
  {
    rect->h += rect->y;
    rect->y = 0;
  }
  if ((rect->y + rect->h) > sdlscrn->h)
    rect->h = (sdlscrn->h - rect->y);

  if ((rect->w > 0) && (rect->h > 0))
    SDL_UpdateRects(sdlscrn, 1, rect);
  else
  {
    rect->x = 0;
    rect->y = 0;
    rect->w = 0;
    rect->h = 0;
  }
}


/*-----------------------------------------------------------------------*/
/*
  Maps an SDL_Color to the screen format.
*/
Uint32 SDLGui_MapColor(SDL_Color *color)
{
  return SDL_MapRGB(sdlscrn->format, color->r, color->g, color->b);
}


/*-----------------------------------------------------------------------*/
/*
  Refresh display to reflect an object change.
*/
void SDLGui_RefreshObj(SGOBJ *dlg, int objnum)
{
  SDL_Rect coord;

  SDLGui_ObjFullCoord(dlg, objnum, &coord);

  SCRLOCK;
  SDLGui_UpdateRect(&coord);
  SCRUNLOCK;
}


/*-----------------------------------------------------------------------*/
/*
  Draw a text string.
*/
void SDLGui_Text(int x, int y, const char *txt, SDL_Color *col)
{
  int i;
  char c;
  SDL_Rect sr, dr;

  SDL_SetColors(fontgfx, col, 1, 1);

  SCRLOCK;
  for (i = 0 ; txt[i] != 0 ; i++)
  {
    c = txt[i];
    sr.x = fontwidth * (c % 16);
    sr.y = fontheight * (c / 16);
    sr.w = fontwidth;
    sr.h = fontheight;

    dr.x = x + (fontwidth * i);
    dr.y = y;
    dr.w = fontwidth;
    dr.h = fontheight;

    SDL_BlitSurface(fontgfx, &sr, sdlscrn, &dr);
  }
  SCRUNLOCK;
}


/*-----------------------------------------------------------------------*/
/*
  Draw a dialog text object.
*/
void SDLGui_DrawText(SGOBJ *tdlg, int objnum)
{
  SDL_Rect coord;
  SDL_Color *textc, *backgroundc;

  if (tdlg[objnum].state & SG_SELECTED)
  {
    textc       = whitec;
    backgroundc = darkgreyc;
  }
  else if (tdlg[objnum].state & SG_DISABLED)
  {
    textc       = darkgreyc;
    backgroundc = greyc;
  }
  else
  {
    textc       = blackc;
    backgroundc = greyc;
  }
  
  SDLGui_ObjCoord(tdlg, objnum, &coord);
  SDL_FillRect(sdlscrn, &coord, SDLGui_MapColor(backgroundc));
  SDLGui_Text(coord.x, coord.y, tdlg[objnum].txt, textc);
}


/*-----------------------------------------------------------------------*/
/*
  Draw an edit field object.
*/
void SDLGui_DrawEditField(SGOBJ *edlg, int objnum)
{
  SDL_Rect coord;
  SDL_Color *textc;

  if (edlg[objnum].state & SG_DISABLED)
    textc = darkgreyc;
  else
    textc = blackc;

  SDLGui_ObjCoord(edlg, objnum, &coord);
  coord.w += 1;
  SDL_FillRect(sdlscrn, &coord, SDLGui_MapColor(greyc));
  SDLGui_Text(coord.x, coord.y, edlg[objnum].txt, textc);

  // Draw a line below.
  coord.y = coord.y + coord.h;
  coord.h = 1;
  SDL_FillRect(sdlscrn, &coord, SDLGui_MapColor(darkgreyc));
}


/*-----------------------------------------------------------------------*/
/*
  Draw or erase cursor.
*/
void SDLGui_DrawCursor(SGOBJ *dlg, cursor_state *cursor)
{
  if (cursor->object != -1)
  {
    SDL_Rect coord;
    SDL_Color *cursorc;

    SDLGui_DrawEditField(dlg, cursor->object);

    if (cursor->blink_state)
      cursorc = blackc;
    else
      cursorc = greyc;

    SDLGui_ObjCoord(dlg, cursor->object, &coord);
    coord.x += (cursor->position * fontwidth);
    coord.w = 1;
    SDL_FillRect(sdlscrn, &coord, SDLGui_MapColor(cursorc));

    SDLGui_RefreshObj(dlg, cursor->object);
  }
}


/*-----------------------------------------------------------------------*/
/*
  Draw a 3D effect around a given rectangle.
  Rectangle is updated to the full size of the new object.
*/
void SDLGui_Draw3DAround(SDL_Rect *coord, SDL_Color *upleftc, SDL_Color *downrightc, SDL_Color *cornerc, int width)
{
  SDL_Rect rect;
  int i;
  Uint32 upleftcol    = SDLGui_MapColor(upleftc);
  Uint32 downrightcol = SDLGui_MapColor(downrightc);
  Uint32 cornercol    = SDLGui_MapColor(cornerc);

  SCRLOCK;

  for ( i = 1 ; i <= width ; i++)
  {
    rect.x = coord->x - i;
    rect.y = coord->y - i;
    rect.w = coord->w + (i * 2) - 1;
    rect.h = 1;
    SDL_FillRect(sdlscrn, &rect, upleftcol);

    rect.x = coord->x - i;
    rect.y = coord->y - i;
    rect.w = 1;
    rect.h = coord->h + (i * 2) - 1;
    SDL_FillRect(sdlscrn, &rect, upleftcol);

    rect.x = coord->x - i + 1;
    rect.y = coord->y + coord->h - 1 + i;
    rect.w = coord->w + (i * 2) - 1;
    rect.h = 1;
    SDL_FillRect(sdlscrn, &rect, downrightcol);

    rect.x = coord->x + coord->w - 1 + i;
    rect.y = coord->y - i + 1;
    rect.w = 1;
    rect.h = coord->h + (i * 2) - 1;
    SDL_FillRect(sdlscrn, &rect, downrightcol);

    rect.x = coord->x + coord->w + i - 1;
    rect.y = coord->y - i;
    rect.w = 1;
    rect.h = 1;
    SDL_FillRect(sdlscrn, &rect, cornercol);

    rect.x = coord->x - i;
    rect.y = coord->y + coord->h + i - 1;
    rect.w = 1;
    rect.h = 1;
    SDL_FillRect(sdlscrn, &rect, cornercol);
  }

  SCRUNLOCK;

  coord->x -= width;
  coord->y -= width;
  coord->w += (width * 2);
  coord->h += (width * 2);
}


/*-----------------------------------------------------------------------*/
/*
  Draw a colored box around a given rectangle.
  Rectangle is updated to the full size of the new object.
*/
void SDLGui_DrawBoxAround(SDL_Rect *coord, SDL_Color *color, int width)
{
  SDL_Rect rect;
  Uint32 col = SDLGui_MapColor(color);

  SCRLOCK;

  rect.x = coord->x - width;
  rect.y = coord->y - width;
  rect.w = coord->w + (width * 2);
  rect.h = width;
  SDL_FillRect(sdlscrn, &rect, col);

  rect.x = coord->x - width;
  rect.y = coord->y - width;
  rect.w = width;
  rect.h = coord->h + (width * 2);
  SDL_FillRect(sdlscrn, &rect, col);

  rect.x = coord->x + coord->w;
  rect.y = coord->y - width;
  rect.w = width;
  rect.h = coord->h + (width * 2);
  SDL_FillRect(sdlscrn, &rect, col);

  rect.x = coord->x - width;
  rect.y = coord->y + coord->h;
  rect.w = coord->w + (width * 2);
  rect.h = width;
  SDL_FillRect(sdlscrn, &rect, col);

  SCRUNLOCK;

  coord->x -= width;
  coord->y -= width;
  coord->w += (width * 2);
  coord->h += (width * 2);
}


/*-----------------------------------------------------------------------*/
/*
  Draw a 3D box with given attributes.
*/
void SDLGui_Draw3DBox(SDL_Rect *coord,
                      SDL_Color *backgroundc,
                      SDL_Color *inboxc,
                      SDL_Color *upleftc,
                      SDL_Color *downrightc,
                      SDL_Color *outboxc,
                      int widthbackground,
                      int widthinbox,
                      int width3D1,
                      int width3D2,
                      int widthoutbox)
{
  SDL_Rect rect;

  SCRLOCK;

  // Draw background
  rect.x = coord->x - widthbackground;
  rect.y = coord->y - widthbackground;
  rect.w = coord->w + (widthbackground * 2);
  rect.h = coord->h + (widthbackground * 2);
  SDL_FillRect(sdlscrn, &rect, SDLGui_MapColor(backgroundc));

  SCRUNLOCK;

  // Update coords
  coord->x -= widthbackground;
  coord->y -= widthbackground;
  coord->w += (widthbackground * 2);
  coord->h += (widthbackground * 2);

  if (widthinbox > 0)
    SDLGui_DrawBoxAround(coord, inboxc, widthinbox);

  if (width3D1 > 0)
    SDLGui_Draw3DAround(coord, upleftc, downrightc, backgroundc, width3D1);

  if (width3D2 > 0)
    SDLGui_Draw3DAround(coord, downrightc, upleftc, backgroundc, width3D2);

  if (widthoutbox > 0)
    SDLGui_DrawBoxAround(coord, outboxc, widthoutbox);
}


/*-----------------------------------------------------------------------*/
/*
  Draw a dialog box object.
*/
void SDLGui_DrawBox(SGOBJ *bdlg, int objnum)
{
  SDL_Rect coord;
  SDL_Color *my_blackc;
  SDL_Color *upleftc, *downrightc;

  SDLGui_ObjCoord(bdlg, objnum, &coord);

  // Modify box drawing according to object state
  if (bdlg[objnum].state & SG_DISABLED)
    my_blackc = darkgreyc;
  else
    my_blackc = blackc;

  if (bdlg[objnum].state & SG_SELECTED)
  {
    upleftc    = darkgreyc;
    downrightc = whitec;
  }
  else
  {
    upleftc    = whitec;
    downrightc = darkgreyc;
  }

  // Draw box according to object flags
  switch (bdlg[objnum].flags & (SG_SELECTABLE | SG_DEFAULT | SG_BACKGROUND))
  {
    case (SG_SELECTABLE | SG_DEFAULT | SG_BACKGROUND):
    case (SG_SELECTABLE | SG_DEFAULT):
      SDLGui_Draw3DBox(&coord,
                       greyc, NULL, upleftc, downrightc, my_blackc,
                       1, 0, 1, 0, 2);
      break;
    case (SG_SELECTABLE | SG_BACKGROUND):
    case SG_SELECTABLE:
      SDLGui_Draw3DBox(&coord,
                       greyc, NULL, upleftc, downrightc, my_blackc,
                       1, 0, 1, 0, 1);
      break;
    case (SG_DEFAULT | SG_BACKGROUND):
    case SG_BACKGROUND:
      SDLGui_Draw3DBox(&coord,
                       greyc, my_blackc, upleftc, downrightc, darkgreyc,
                       0, 2, 3, 0, 1);
      break;
    case SG_DEFAULT:
    case 0:
      SDLGui_Draw3DBox(&coord,
                       greyc, NULL, upleftc, downrightc, NULL,
                       3, 0, 1, 1, 0);
      break;
  }
}


/*-----------------------------------------------------------------------*/
/*
  Draw a normal button.
*/
void SDLGui_DrawButton(SGOBJ *bdlg, int objnum)
{
  SDL_Rect coord;
  int x, y;
  SDL_Color *textc;

  SDLGui_ObjCoord(bdlg, objnum, &coord);

  x = coord.x + ((coord.w - (strlen(bdlg[objnum].txt) * fontwidth)) / 2);
  y = coord.y + ((coord.h - fontheight) / 2);

  if (bdlg[objnum].state & SG_SELECTED)
  {
    x += 1;
    y += 1;
  }

  if (bdlg[objnum].state & SG_DISABLED)
    textc = darkgreyc;
  else
    textc = blackc;

  SDLGui_DrawBox(bdlg, objnum);
  SDLGui_Text(x, y, bdlg[objnum].txt, textc);
}


/*-----------------------------------------------------------------------*/
/*
  Draw a dialog check box object state.
*/
void SDLGui_DrawCheckBoxState(SGOBJ *cdlg, int objnum)
{
  Uint32 grey = SDLGui_MapColor(greyc);
  SDL_Rect coord;
  char str[2];
  SDL_Color *textc;

  SDLGui_ObjCoord(cdlg, objnum, &coord);

  if (cdlg[objnum].flags & SG_RADIO)
  {
    if (cdlg[objnum].state & SG_SELECTED)
      str[0]=SGCHECKBOX_RADIO_SELECTED;
    else
      str[0]=SGCHECKBOX_RADIO_NORMAL;
  }
  else
  {
    if (cdlg[objnum].state & SG_SELECTED)
      str[0]=SGCHECKBOX_SELECTED;
    else
      str[0]=SGCHECKBOX_NORMAL;
  }

  if (cdlg[objnum].state & SG_DISABLED)
    textc = darkgreyc;
  else
    textc = blackc;

  str[1]='\0';

  coord.w = fontwidth;
  coord.h = fontheight;

  if (cdlg[objnum].flags & SG_BUTTON_RIGHT)
    coord.x += ((strlen(cdlg[objnum].txt) + 1) * fontwidth);

  SDL_FillRect(sdlscrn, &coord, grey);
  SDLGui_Text(coord.x, coord.y, str, textc);
}


/*-----------------------------------------------------------------------*/
/*
  Draw a dialog check box object.
*/
void SDLGui_DrawCheckBox(SGOBJ *cdlg, int objnum)
{
  SDL_Rect coord;
  SDL_Color *textc;

  SDLGui_ObjCoord(cdlg, objnum, &coord);

  if (!(cdlg[objnum].flags&SG_BUTTON_RIGHT))
    coord.x += (fontwidth * 2);

  if (cdlg[objnum].state & SG_DISABLED)
    textc = darkgreyc;
  else
    textc = blackc;

  SDLGui_Text(coord.x, coord.y, cdlg[objnum].txt, textc);
  SDLGui_DrawCheckBoxState(cdlg, objnum);
}


/*-----------------------------------------------------------------------*/
/*
  Draw a dialog popup button object.
*/
void SDLGui_DrawPopupButton(SGOBJ *pdlg, int objnum)
{
  SDL_Rect coord;
  const char *downstr = "\x02";
  SDL_Color *textc;

  if (pdlg[objnum].state & SG_DISABLED)
    textc = darkgreyc;
  else
    textc = blackc;

  SDLGui_DrawBox(pdlg, objnum);

  SDLGui_ObjCoord(pdlg, objnum, &coord);

  SDLGui_Text(coord.x, coord.y, pdlg[objnum].txt, textc);
  SDLGui_Text(coord.x+coord.w-fontwidth, coord.y, downstr, textc);
}


/*-----------------------------------------------------------------------*/
/*
  Draw an object.
*/
void SDLGui_DrawObject(SGOBJ *dlg, int objnum)
{
  switch (dlg[objnum].type)
  {
    case SGBOX:
      SDLGui_DrawBox(dlg, objnum);
      break;
    case SGTEXT:
      SDLGui_DrawText(dlg, objnum);
      break;
    case SGEDITFIELD:
      SDLGui_DrawEditField(dlg, objnum);
      break;
    case SGBUTTON:
      SDLGui_DrawButton(dlg, objnum);
      break;
    case SGCHECKBOX:
      SDLGui_DrawCheckBox(dlg, objnum);
      break;
    case SGPOPUP:
      SDLGui_DrawPopupButton(dlg, objnum);
      break;
  }
}


/*-----------------------------------------------------------------------*/
/*
  Draw a whole dialog.
*/
void SDLGui_DrawDialog(SGOBJ *dlg)
{
  int i;

  // Store dialog coordinates
  SDLGui_ObjFullCoord(dlg, 0, &DialogRect);

  for (i = 0 ; dlg[i].type != -1 ; i++)
  {
    if (dlg[i].state & SG_HIDDEN) continue;
    SDLGui_DrawObject(dlg, i);
  }
  SDLGui_RefreshObj(dlg, 0);
}


/*-----------------------------------------------------------------------*/
/*
  Search default object in a dialog.
*/
int SDLGui_FindDefaultObj(SGOBJ *dlg)
{
  int i = 0;

  while (dlg[i].type != -1)
  {
    if (dlg[i].flags & SG_DEFAULT)
      return i;
    i++;
  }

  return -1;
}


/*-----------------------------------------------------------------------*/
/*
  Search an object at given coordinates.
*/
int SDLGui_FindObj(SGOBJ *dlg, int fx, int fy)
{
  SDL_Rect coord;
  int end, i;
  int ob = -1;
  
  // Search end object in dialog
  i = 0;
  while (dlg[i++].type != -1);
  end = i;

  // Now check each object
  for (i = end-1 ; i >= 0 ; i--)
  {
    SDLGui_ObjFullCoord(dlg, i, &coord);

    if(fx >= coord.x &&
       fy >= coord.y &&
       fx < (coord.x + coord.w) &&
       fy < (coord.y + coord.h))
    {
      if (dlg[i].state & (SG_HIDDEN | SG_DISABLED)) continue;
      ob = i;
      break;
    }
  }

  return ob;
}


/*-----------------------------------------------------------------------*/
/*
  A radio object has been selected. Let's deselect any other in his group.
*/
void SDLGui_SelectRadioObject(SGOBJ *dlg, int clicked_obj)
{
  int obj;

  // Find first radio object in this group
  obj = clicked_obj;
  while (dlg[--obj].flags & SG_RADIO);

  // Update state
  while (dlg[++obj].flags & SG_RADIO)
  {
    // This code scan every object in the group. This allows to solve cases
    // where multiple objects where selected in the group by clicking one.
    if ((obj != clicked_obj) && (dlg[obj].state & SG_SELECTED))
    {
      // Deselect this radio button
      dlg[obj].state &= ~SG_SELECTED;
      SDLGui_DrawObject(dlg, obj);
      SDLGui_RefreshObj(dlg, obj);
    }
  }
}


/*-----------------------------------------------------------------------*/
/*
  Update clicked object state depending on given mouse coordinates.
  Returns true if the mouse is over the object, false otherwise.
*/
bool SDLGui_UpdateObjState(SGOBJ *dlg, int clicked_obj, int original_state,
                           int x, int y)
{
  int obj;

  obj = SDLGui_FindObj(dlg, x, y);

  // Special case : user clicked on an already selected radio object
  // do not modify its state.
  // We handle it here because it allows to exit if the object is SG_EXIT or
  // SG_TOUCHEXIT without any additional test.
  if ((dlg[clicked_obj].flags & SG_RADIO) && (original_state & SG_SELECTED))
    return (obj == clicked_obj);

  if (((obj != clicked_obj) &&
       (dlg[clicked_obj].state != original_state)) ||
      ((obj == clicked_obj) &&
       (dlg[clicked_obj].state == original_state)))
  {
    if (dlg[clicked_obj].flags & SG_SELECTABLE)
    {
      dlg[clicked_obj].state ^= SG_SELECTED;
      SDLGui_DrawObject(dlg, clicked_obj);
      SDLGui_RefreshObj(dlg, clicked_obj);
    }
  }

  return (obj == clicked_obj);
}


/*-----------------------------------------------------------------------*/
/*
  Search edit field in a dialog.
*/
int SDLGui_FindEditField(SGOBJ *dlg, int objnum, int mode)
{
  int i, j;

  switch (mode)
  {
    case SG_FIRST_EDITFIELD:
      i = 0;
      while (dlg[i].type != -1)
      {
        if ((dlg[i].type == SGEDITFIELD) &&
            ((dlg[i].state & (SG_HIDDEN | SG_DISABLED)) == 0))
          return i;
        i++;
      }
      break;

    case SG_PREVIOUS_EDITFIELD:
      i = objnum - 1;
      while (i >= 0)
      {
        if ((dlg[i].type == SGEDITFIELD) &&
            ((dlg[i].state & (SG_HIDDEN | SG_DISABLED)) == 0))
          return i;
        i--;
      }
      break;

    case SG_NEXT_EDITFIELD:
      i = objnum + 1;
      while (dlg[i].type != -1)
      {
        if ((dlg[i].type == SGEDITFIELD) &&
            ((dlg[i].state & (SG_HIDDEN | SG_DISABLED)) == 0))
          return i;
        i++;
      }
      break;

    case SG_LAST_EDITFIELD:
      i = objnum + 1;
      j = -1;
      while (dlg[i].type != -1)
      {
        if ((dlg[i].type == SGEDITFIELD) &&
            ((dlg[i].state & (SG_HIDDEN | SG_DISABLED)) == 0))
          j = i;
        i++;
      }
      if (j != -1)
        return j;
      break;
  }

  return objnum;
}


/*-----------------------------------------------------------------------*/
/*
  Move cursor to another edit field.
*/
void SDLGui_MoveCursor(SGOBJ *dlg, cursor_state *cursor, int mode)
{
  int new_object;

  new_object = SDLGui_FindEditField(dlg, cursor->object, mode);

  if (new_object != cursor->object)
  {
    /* Erase old cursor */
    cursor->blink_state = false;
    SDLGui_DrawCursor(dlg, cursor);

    cursor->object = new_object;
    cursor->position = strlen(dlg[new_object].txt);
  }
  else
  {
    /* We stay in the same field */
    /* Move cursor to begin or end of text depending on mode */
    switch (mode)
    {
      case SG_FIRST_EDITFIELD:
      case SG_PREVIOUS_EDITFIELD:
        cursor->position = 0;
        break;

      case SG_NEXT_EDITFIELD:
      case SG_LAST_EDITFIELD:
        cursor->position = strlen(dlg[new_object].txt);
        break;
    }
  }
}


/*-----------------------------------------------------------------------*/
/*
  Handle mouse clicks on edit fields.
*/
void SDLGui_ClickEditField(SGOBJ *dlg, cursor_state *cursor, int clicked_obj, int x)
{
  SDL_Rect coord;
  int i, j;

  /* Erase old cursor */
  cursor->blink_state = false;
  SDLGui_DrawCursor(dlg, cursor);

  SDLGui_ObjFullCoord(dlg, clicked_obj, &coord);
  i = (x - coord.x + (fontwidth / 2)) / fontwidth;
  j = strlen(dlg[clicked_obj].txt);

  cursor->object = clicked_obj;
  cursor->position = MIN(i, j);
  cursor->blink_state = true;
  cursor->blink_counter = 0;
  SDLGui_DrawCursor(dlg, cursor);
}


/*-----------------------------------------------------------------------*/
/*
  Handle mouse clicks.
*/
int SDLGui_MouseClick(SGOBJ *dlg, int fx, int fy, cursor_state *cursor)
{
  int clicked_obj;
  int return_obj = -1;
  int original_state = 0;
  int x, y;

  clicked_obj = SDLGui_FindObj(dlg, fx, fy);

  if (clicked_obj >= 0)
  {
    original_state = dlg[clicked_obj].state;
    SDLGui_UpdateObjState(dlg, clicked_obj, original_state, fx, fy);

    if (dlg[clicked_obj].flags & SG_TOUCHEXIT)
    {
      return_obj = clicked_obj;
      clicked_obj = -1;
    }
  }

  while (clicked_obj >= 0)
  {
    SDL_Event evnt;
    // SDL_PumpEvents() - not necessary, the main check_event thread calls it
    if (SDL_PeepEvents(&evnt, 1, SDL_GETEVENT, SDL_EVENTMASK(SDL_USEREVENT)))
    {
      switch (evnt.user.code)
      {
        case SDL_USEREVENT:
          // a signal that resolution has changed
          // Restore clicked object original state
          dlg[clicked_obj].state = original_state;

          // re-draw dialog
          SDLGui_DrawDialog(dlg);

          // Exit from mouse click handling.
          clicked_obj = -1;
          break;

        case SDL_MOUSEBUTTONUP:
          x = reinterpret_cast<intptr>(evnt.user.data1);
          y = reinterpret_cast<intptr>(evnt.user.data2);
          if (SDLGui_UpdateObjState(dlg, clicked_obj, original_state, x, y))
          {
            // true if mouse button is released over clicked object.
            // If applicable, the object has been selected by
            // SDLGui_UpdateObjState(). Let's do additional handling here.

            // Exit if object is an SG_EXIT one.
            if (dlg[clicked_obj].flags & SG_EXIT)
              return_obj = clicked_obj;

            // If it's a SG_RADIO object, deselect other objects in his group.
            if (dlg[clicked_obj].flags & SG_RADIO)
              SDLGui_SelectRadioObject(dlg, clicked_obj);

            if (dlg[clicked_obj].type == SGEDITFIELD)
              SDLGui_ClickEditField(dlg, cursor, clicked_obj, x);
          }

          // Exit from mouse click handling.
          clicked_obj = -1;

          break;
      }
    }
    else
    {
      // No special event occured.
      // Update object state according to mouse coordinates.
      SDL_GetMouseState(&x, &y);
      SDLGui_UpdateObjState(dlg, clicked_obj, original_state, x, y);

      // Wait a little to avoid eating CPU.
      SDL_Delay(100);
    }
  }

  return return_obj;
}


/*-----------------------------------------------------------------------*/
/*
  Handle key press.
*/
int SDLGui_KeyPress(SGOBJ *dlg, int keysym, int mod, cursor_state *cursor)
{
  int return_obj = -1;
  int obj;

  if (cursor->object != -1)
  {
    switch(keysym)
    {
      case SDLK_RETURN:
      case SDLK_KP_ENTER:
        break;

      case SDLK_BACKSPACE:
        if (cursor->position > 0)
        {
          memmove(&dlg[cursor->object].txt[cursor->position-1],
                  &dlg[cursor->object].txt[cursor->position],
                  strlen(&dlg[cursor->object].txt[cursor->position])+1);
          cursor->position--;
        }
        break;

      case SDLK_DELETE:
        if(cursor->position < (int)strlen(dlg[cursor->object].txt))
        {
          memmove(&dlg[cursor->object].txt[cursor->position],
                  &dlg[cursor->object].txt[cursor->position+1],
                  strlen(&dlg[cursor->object].txt[cursor->position+1])+1);
        }
        break;

      case SDLK_LEFT:
        if (cursor->position > 0)
          cursor->position--;
        break;

      case SDLK_RIGHT:
        if (cursor->position < (int)strlen(dlg[cursor->object].txt))
          cursor->position++;
        break;

      case SDLK_DOWN:
        SDLGui_MoveCursor(dlg, cursor, SG_NEXT_EDITFIELD);
        break;

      case SDLK_UP:
        SDLGui_MoveCursor(dlg, cursor, SG_PREVIOUS_EDITFIELD);
        break;

      case SDLK_TAB:
        if (mod & KMOD_SHIFT)
          SDLGui_MoveCursor(dlg, cursor, SG_PREVIOUS_EDITFIELD);
        else
          SDLGui_MoveCursor(dlg, cursor, SG_NEXT_EDITFIELD);
        break;

      case SDLK_HOME:
        if (mod & KMOD_CTRL)
          SDLGui_MoveCursor(dlg, cursor, SG_FIRST_EDITFIELD);
        else
          cursor->position = 0;
        break;

      case SDLK_END:
        if (mod & KMOD_CTRL)
          SDLGui_MoveCursor(dlg, cursor, SG_LAST_EDITFIELD);
        else
          cursor->position = strlen(dlg[cursor->object].txt);
        break;

      default:
        if ((keysym >= SDLK_KP0) && (keysym <= SDLK_KP9))
        {
          // map numpad numbers to normal numbers
          keysym -= (SDLK_KP0 - SDLK_0);
        }
        /* If it is a "good" key then insert it into the text field */
        if ((keysym >= SDLK_SPACE) && (keysym < SDLK_KP0))
        {
          if (strlen(dlg[cursor->object].txt) < dlg[cursor->object].w)
          {
            memmove(&dlg[cursor->object].txt[cursor->position+1],
                    &dlg[cursor->object].txt[cursor->position],
                    strlen(&dlg[cursor->object].txt[cursor->position])+1);
            if (mod & KMOD_SHIFT)
              dlg[cursor->object].txt[cursor->position] = toupper(keysym);
            else
              dlg[cursor->object].txt[cursor->position] = keysym;
            cursor->position += 1;
          }
        }
        break;
    }
  }

  switch(keysym)
  {
    case SDLK_RETURN:
    case SDLK_KP_ENTER:
      obj = SDLGui_FindDefaultObj(dlg);
      if (obj >= 0)
      {
        dlg[obj].state ^= SG_SELECTED;
        SDLGui_DrawObject(dlg, obj);
        SDLGui_RefreshObj(dlg, obj);
        if (dlg[obj].flags & (SG_EXIT | SG_TOUCHEXIT))
        {
          return_obj = obj;
          SDL_Delay(300);
        }
      }
      break;
  }

  // Force cursor display. Should ease text input.
  cursor->blink_state = true;
  cursor->blink_counter = 0;
  // Redraw current edit field...
  SDLGui_DrawCursor(dlg, cursor);

  return return_obj;
}


/*-----------------------------------------------------------------------*/
/*
  Used to update screen while GUI is opened. Return a list of rectangles that
  covers the screen without overlaping the current dialog.
*/
SDL_Rect *SDLGui_GetFirstBackgroundRect(void)
{
  // Reset counter...
  BackgroundRectCounter = SG_BCKGND_RECT_BEGIN;
  // And returns first rectangle
  return SDLGui_GetNextBackgroundRect();
}


/*-----------------------------------------------------------------------*/
/*
  Returns next rectangle to be redrawn to update screen or NULL if we reached
  the end of the list.
  This code is "flying dialog" ready :)
  It will need some updating if we implement popup buttons handled by sdlgui,
  as the popup could be higher than the root box...
  I used some recursivity here to simplify the code.
*/
SDL_Rect *SDLGui_GetNextBackgroundRect(void)
{
  SDL_Rect *return_rect = NULL;

  switch (BackgroundRectCounter)
  {
    case SG_BCKGND_RECT_END:
      // Nothing to do : return_rect is already initialized to NULL.
      break;

    case SG_BCKGND_RECT_BEGIN:
      if (DialogRect.w == 0)
      {
        // The dialog is not drawn yet...
        // Let's redraw the full screen.
      	BackgroundRect.x = 0;
      	BackgroundRect.y = 0;
      	BackgroundRect.w = sdlscrn->w;
      	BackgroundRect.h = sdlscrn->h;
        return_rect = &BackgroundRect;
        // We reached the end of the list.
        BackgroundRectCounter = SG_BCKGND_RECT_END;
      }
      else
      {
        BackgroundRectCounter = SG_BCKGND_RECT_TOP;
        return_rect = SDLGui_GetNextBackgroundRect();
      }
      break;

    case SG_BCKGND_RECT_TOP:
      BackgroundRectCounter = SG_BCKGND_RECT_LEFT;
      if (DialogRect.y > 0)
      {
      	BackgroundRect.x = 0;
      	BackgroundRect.y = 0;
      	BackgroundRect.w = sdlscrn->w;
      	BackgroundRect.h = DialogRect.y;
        return_rect = &BackgroundRect;
      }
      else
        return_rect = SDLGui_GetNextBackgroundRect();
      break;

    case SG_BCKGND_RECT_LEFT:
      BackgroundRectCounter = SG_BCKGND_RECT_RIGHT;
      if (DialogRect.x > 0)
      {
        BackgroundRect.x = 0;
        BackgroundRect.y = (DialogRect.y > 0) ? DialogRect.y : 0;
        BackgroundRect.w = DialogRect.x;
        BackgroundRect.h =
          ((DialogRect.y + DialogRect.h) < (int)sdlscrn->h) ?
          (DialogRect.h + DialogRect.y - BackgroundRect.y) :
          (sdlscrn->h - DialogRect.y);
        return_rect = &BackgroundRect;
      }
      else
        return_rect = SDLGui_GetNextBackgroundRect();
      break;

    case SG_BCKGND_RECT_RIGHT:
      BackgroundRectCounter = SG_BCKGND_RECT_BOTTOM;
      if ((DialogRect.x + DialogRect.w) < (int)sdlscrn->w)
      {
        BackgroundRect.x = DialogRect.x + DialogRect.w;
        BackgroundRect.y = (DialogRect.y > 0) ? DialogRect.y : 0;
        BackgroundRect.w = sdlscrn->w - (DialogRect.x + DialogRect.w);
        BackgroundRect.h =
          ((DialogRect.y + DialogRect.h) < (int)sdlscrn->w) ?
          (DialogRect.h + DialogRect.y - BackgroundRect.y) :
          (sdlscrn->h - DialogRect.y);
        return_rect = &BackgroundRect;
      }
      else
        return_rect = SDLGui_GetNextBackgroundRect();
      break;

    case SG_BCKGND_RECT_BOTTOM:
      BackgroundRectCounter = SG_BCKGND_RECT_END;
      if ((DialogRect.y + DialogRect.h) < (int)sdlscrn->h)
      {
        // Bottom
        BackgroundRect.x = 0;
        BackgroundRect.y = DialogRect.y + DialogRect.h;
        BackgroundRect.w = sdlscrn->w;
        BackgroundRect.h = sdlscrn->h - (DialogRect.y + DialogRect.h);
        return_rect = &BackgroundRect;
      }
      else
        return_rect = SDLGui_GetNextBackgroundRect();
      break;
  }

  return return_rect;
}

SDL_Event getEvent(SGOBJ *dlg, cursor_state *cursor)
{
  int i = 0;
  while(1) {
    SDL_Event evnt;
//    fprintf(stderr, "Debug Before Peep events\n");
    if (SDL_PeepEvents(&evnt, 1, SDL_GETEVENT, SDL_EVENTMASK(SDL_USEREVENT)))
    {
	fprintf(stderr, "Debug Peep events %d\n",i++);
      SDL_Event e;
      switch(evnt.user.code)
      {
      	case SDL_KEYDOWN:
	case SDL_KEYUP:
	  e.type = evnt.user.code;
          e.key.keysym.sym = (SDLKey)reinterpret_cast<uintptr>(evnt.user.data1);
          e.key.keysym.mod = (SDLMod)reinterpret_cast<uintptr>(evnt.user.data2);
	  return e;

        case SDL_MOUSEBUTTONDOWN:
	case SDL_MOUSEBUTTONUP:
	  e.type = evnt.user.code;
	if (evnt.user.code == SDL_MOUSEBUTTONDOWN)
		fprintf(stderr, "Debug mouse down\n");
	else
		fprintf(stderr, "Debug mouse down\n");
          e.button.x = reinterpret_cast<intptr>(evnt.user.data1);
          e.button.y = reinterpret_cast<intptr>(evnt.user.data2);
	  return e;

        case SDL_USEREVENT:
          // a signal that resolution has changed
	  if (dlg != NULL)
              SDLGui_DrawDialog(dlg);	// re-draw dialog
          break;
      }
    }
    else
    {
      // No special event occured.
      // Wait a little to avoid eating CPU.
      SDL_Delay(50);
      if (cursor != NULL) {
          cursor->blink_counter++;
          if (cursor->blink_counter >= 10) {
            cursor->blink_counter = 0;
            cursor->blink_state = !cursor->blink_state;
	    if (dlg != NULL)
              SDLGui_DrawCursor(dlg, cursor);
          }
	}
    }
  }
}

/*-----------------------------------------------------------------------*/
/*
  Show and process a dialog. Returns the button number that has been
  pressed. Does NOT handle SDL_QUIT - you must handle it before you
  pass the input event to the SDL GUI.
*/
int SDLGui_DoDialog(SGOBJ *dlg)
{
  int return_obj = -1;
  int obj;
  int x, y;
  // int keysym, mod;
  cursor_state cursor;

  // Is the left mouse button still pressed? Yes -> Handle TOUCHEXIT objects here
  bool stillPressed = (SDL_GetMouseState(&x, &y) & SDL_BUTTON(1));
  obj = SDLGui_FindObj(dlg, x, y);
  if (stillPressed && (obj >= 0) && (dlg[obj].flags & SG_TOUCHEXIT))
  {
    // Mouse button is pressed over a TOUCHEXIT Button
    // Toogle its state before drawing anything (it has been deselected before).
    dlg[obj].state ^= SG_SELECTED;

    return_obj = obj;
  }

  cursor.object = SDLGui_FindEditField(dlg, -1, SG_FIRST_EDITFIELD);
  cursor.position = (cursor.object != -1) ? strlen(dlg[cursor.object].txt) : 0;
  cursor.blink_counter = 0;
  cursor.blink_state = true;

  SDLGui_DrawDialog(dlg);

  /* The main loop */
  while (return_obj < 0)
  {
    fprintf(stderr, "Debug SDL main loop\n");
    SDL_Event evnt = getEvent(dlg, &cursor);
    fprintf(stderr, "Debug SDL main loop got event\n");
    switch(evnt.type)
      {
      	case SDL_KEYDOWN:
          return_obj = SDLGui_KeyPress(dlg, evnt.key.keysym.sym, evnt.key.keysym.mod, &cursor);
      	  break;

        case SDL_MOUSEBUTTONDOWN:
          return_obj = SDLGui_MouseClick(dlg, evnt.button.x, evnt.button.y, &cursor);
          break;
      }
  }
  fprintf(stderr, "Debug SDL main loop finished\n");
  if (dlg[return_obj].type == SGBUTTON)
  {
    // Deselect button...
    // BUG: This should be caller responsibility
    dlg[return_obj].state ^= SG_SELECTED;
  }

  return return_obj;
}
