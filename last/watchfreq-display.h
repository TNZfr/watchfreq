typedef struct
{
  int  Row,Col;
  char *Buffer;
  char String[2][32];
} CELL;

typedef struct
{
  int Current;
  int NbCell;
  int MaxCell;
  CELL *Cell;
} DISP;

void  CEL_PrintKhz     (CELL *Cell, char *Color, long Freq);
void  CEL_PrintVal     (CELL *Cell, long Value);
void CEL_PrintValColor (CELL *Cell, long Value, char *Color);
void  CEL_Print2Val    (CELL *Cell, long Value1,  long Value2);

void  DSP_Init      (DISP *Display, int NbCell);
CELL *DSP_CellAlloc (DISP *Display, int Row, int Col);
void  DSP_Free      (DISP *Display);
void  DSP_Refresh   (DISP *Display, int CursorRow);
