
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "watchfreq-display.h"

//------------------------------------------------------------------------------
void CEL_PrintKhz (CELL *Cell, char *Color, long Freq)
{
  if (Freq >= 1000000)
    sprintf (Cell->Buffer,
	     "%c[%sm%3d %03d %03d%c[m",
	     27, Color,
	     Freq/1000000, Freq/1000%1000, Freq%1000,
	     27);
  else
    sprintf (Cell->Buffer,
	     "%c[%sm    %3d %03d%c[m",
	     27, Color,
	     Freq/1000, Freq%1000,
	     27);
}

void CEL_PrintVal (CELL *Cell, long Value)
{
  sprintf (Cell->Buffer, "%15d", Value);
}

void CEL_Print2Val (CELL *Cell, long Value1, long Value2)
{
  sprintf (Cell->Buffer, "%4d / %c[34m%4d%c[m", Value1, 27,Value2,27);
}

//------------------------------------------------------------------------------
void DSP_Init (DISP *Display, int NbCell)
{
  register int i;
  
  memset (Display, 0, sizeof(DISP));
  Display->MaxCell = NbCell;
  Display->Cell    = malloc (NbCell * sizeof(CELL));

  for (i=0; i<NbCell; i++)
    Display->Cell[i].Buffer = Display->Cell[i].String[0];
}

CELL *DSP_CellAlloc (DISP *Display, int Row, int Col)
{
  CELL *CellAlloc;
  
  if (Display->MaxCell == Display->NbCell)
  {
    printf ("DSP_CellAlloc : Not enough Cell (Max=%d)\n",Display->MaxCell);
    exit (1);
  }

  CellAlloc = &Display->Cell[Display->NbCell];
  Display->NbCell ++;

  CellAlloc->Row = Row;
  CellAlloc->Col = Col;

  return CellAlloc;
}

void DSP_Free (DISP *Display)
{
  free (Display->Cell);
  free (Display);
}

//------------------------------------------------------------------------------
void DSP_Refresh (DISP *Display, int CursorRow)
{
  register int i,Prev;
  
  Prev = (Display->Current + 1) % 2;
  
  for (i=0; i<Display->NbCell; i++)
  {
    if (strcmp(Display->Cell[i].String[Display->Current],
	       Display->Cell[i].String[Prev]            ) != 0)
    {
      printf ("%c[%d;%dH%s",
	      27, Display->Cell[i].Row, Display->Cell[i].Col,
	      Display->Cell[i].Buffer);
    }
    Display->Cell[i].Buffer = Display->Cell[i].String[Prev];
  }
  printf ("%c[%dH",27,CursorRow); fflush (stdout);
  Display->Current = Prev;

}
