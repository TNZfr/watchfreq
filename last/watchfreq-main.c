
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "watchfreq-display.h"

#define FREQ_BASE_DIR "/sys/devices/system/cpu/cpufreq"
#define COLOR_RED     "31"
#define COLOR_GREEN   "32"
#define COLOR_ORANGE  "33"
#define COLOR_DEFAULT ""

typedef struct
{
  long Value;
  CELL *Cell;
} ONEFREQ;

typedef struct
{
  ONEFREQ Cur, Min, Max;
} FREQ;

//------------------------------------------------------------------------------
long GetNbCore()
{
  FILE *pfd;
  char Buffer[16] = "";
  
  pfd = popen ("nproc","r");
  fread (Buffer,sizeof(Buffer)-1,1,pfd);
  pclose (pfd);

  return (long)atoi(Buffer);
}

//------------------------------------------------------------------------------
void read_freq (FREQ *Freq, int NbFreq)
{
  register int i;
  char  Buffer[16];
  FILE *fd;
  char  FreqFile[256];

  for (i=0; i<NbFreq; i++)
  {
    sprintf (FreqFile,"%s/policy%d/scaling_cur_freq",FREQ_BASE_DIR,i);
    fd = fopen (FreqFile,"r");
    fread (Buffer,sizeof(Buffer)-1,1,fd);
    fclose(fd);
    
    Freq[i].Cur.Value = (long)atoi (Buffer);

    if (!Freq[i].Min.Value) Freq[i].Min.Value = Freq[i].Cur.Value;
    if (!Freq[i].Max.Value) Freq[i].Max.Value = Freq[i].Cur.Value;
    if ( Freq[i].Cur.Value < Freq[i].Min.Value) Freq[i].Min.Value = Freq[i].Cur.Value;
    if ( Freq[i].Cur.Value > Freq[i].Max.Value) Freq[i].Max.Value = Freq[i].Cur.Value;
  }
}

//------------------------------------------------------------------------------
void DisplayMain (int NbCore)
{
  register int i;
  
  printf ("%c[H%c[2J",27,27);
  printf ("                   Current         Minimum         Maximum\n");
  for (i=0; i<NbCore; i++)
    printf ("Core  %2d :             Khz             Khz             Khz\n",i);
  printf ("\n");
  printf ("Total    :             Khz             Khz             Khz\n");
  printf ("Average  :             Khz             Khz             Khz\n");
  printf ("\n");
  printf ("# Mini   :\n");
  printf ("# Running:\n");
  printf ("# Maxi   :\n");
  printf ("\n");
  printf ("Extr. Min:             Khz             Khz             Khz\n");
  printf ("Extr. Max:             Khz             Khz             Khz\n");
  fflush (stdout);
}

//------------------------------------------------------------------------------
int main (int NbArg, char **Arg)
{
  register int i,boucle;
  int   NbCore   = GetNbCore();
  int   Secondes;

  FREQ *FreqData;
  FREQ  Total, Moyenne;
  FREQ  NbMin, NbInt, NbMax;
  FREQ  TopNbMin, TopNbMax;
  FREQ  ExtrMin, ExtrMax;

  FREQ  ColCur, ColMin, ColMax;

  DISP  Display;

  if (NbArg < 2)
  {
    printf ("\n");
    printf ("Syntaxe : %s NbSec\n", Arg[0]);
    printf ("\n");
    printf ("\tNbSec : Monitoring time in seconds.\n");
    printf ("\n");
    return 1;
  }

  Secondes = (int)atoi(Arg[1]);

  DSP_Init (&Display, (NbCore + 7) * 3 );
  
  FreqData = malloc ( (NbCore + 7) * sizeof(FREQ) );
  for (i=0; i<NbCore; i++)
  {
    FreqData[i].Cur.Cell = DSP_CellAlloc (&Display, i+2, 12);
    FreqData[i].Min.Cell = DSP_CellAlloc (&Display, i+2, 28);
    FreqData[i].Max.Cell = DSP_CellAlloc (&Display, i+2, 44);
  }

  Total.Cur.Cell   = DSP_CellAlloc (&Display, NbCore+3, 12);
  Total.Min.Cell   = DSP_CellAlloc (&Display, NbCore+3, 28);
  Total.Max.Cell   = DSP_CellAlloc (&Display, NbCore+3, 44);
  Moyenne.Cur.Cell = DSP_CellAlloc (&Display, NbCore+4, 12);
  Moyenne.Min.Cell = DSP_CellAlloc (&Display, NbCore+4, 28);
  Moyenne.Max.Cell = DSP_CellAlloc (&Display, NbCore+4, 44);

  NbMin.Cur.Cell = DSP_CellAlloc (&Display, NbCore+6, 12);
  NbMin.Min.Cell = DSP_CellAlloc (&Display, NbCore+6, 28);
  NbMin.Max.Cell = DSP_CellAlloc (&Display, NbCore+6, 44);
  NbInt.Cur.Cell = DSP_CellAlloc (&Display, NbCore+7, 12);
  NbInt.Min.Cell = DSP_CellAlloc (&Display, NbCore+7, 28);
  NbInt.Max.Cell = DSP_CellAlloc (&Display, NbCore+7, 44);
  NbMax.Cur.Cell = DSP_CellAlloc (&Display, NbCore+8, 12);
  NbMax.Min.Cell = DSP_CellAlloc (&Display, NbCore+8, 28);
  NbMax.Max.Cell = DSP_CellAlloc (&Display, NbCore+8, 44);

  ExtrMin.Cur.Cell = DSP_CellAlloc (&Display, NbCore+10, 12);
  ExtrMin.Min.Cell = DSP_CellAlloc (&Display, NbCore+10, 28);
  ExtrMin.Max.Cell = DSP_CellAlloc (&Display, NbCore+10, 44);
  ExtrMax.Cur.Cell = DSP_CellAlloc (&Display, NbCore+11, 12);
  ExtrMax.Min.Cell = DSP_CellAlloc (&Display, NbCore+11, 28);
  ExtrMax.Max.Cell = DSP_CellAlloc (&Display, NbCore+11, 44);

  // Affichage ecran principal
  DisplayMain (NbCore);

  for (boucle=0; boucle < Secondes; boucle ++)
  {
    read_freq  (FreqData, NbCore);

    memset (&ColCur, 0, sizeof(FREQ));
    memset (&ColMin, 0, sizeof(FREQ));
    memset (&ColMax, 0, sizeof(FREQ));

    // definir les maximum et minimum des 3 colonnes
    for (i=0; i<NbCore; i++)
    { 
      if (!ColCur.Min.Value) ColCur.Min.Value = FreqData[i].Cur.Value;
      if (!ColMin.Min.Value) ColMin.Min.Value = FreqData[i].Min.Value;
      if (!ColMax.Min.Value) ColMax.Min.Value = FreqData[i].Max.Value;
     
      if (ColCur.Max.Value < FreqData[i].Cur.Value) ColCur.Max.Value = FreqData[i].Cur.Value;
      if (ColCur.Min.Value > FreqData[i].Cur.Value) ColCur.Min.Value = FreqData[i].Cur.Value;

      if (ColMin.Max.Value < FreqData[i].Min.Value) ColMin.Max.Value = FreqData[i].Min.Value;
      if (ColMin.Min.Value > FreqData[i].Min.Value) ColMin.Min.Value = FreqData[i].Min.Value;

      if (ColMax.Max.Value < FreqData[i].Max.Value) ColMax.Max.Value = FreqData[i].Max.Value;
      if (ColMax.Min.Value > FreqData[i].Max.Value) ColMax.Min.Value = FreqData[i].Max.Value;
    }    
    
    NbMin.Cur.Value = NbMin.Min.Value = NbMin.Max.Value = 0;
    NbMax.Cur.Value = NbMax.Min.Value = NbMax.Max.Value = 0;
    for (i=0; i<NbCore; i++)
    {
      char *Couleur;
      
      Couleur = COLOR_DEFAULT;
      if      (FreqData[i].Cur.Value == ColCur.Min.Value) Couleur=COLOR_GREEN, NbMin.Cur.Value++;
      else if (FreqData[i].Cur.Value == ColCur.Max.Value) Couleur=COLOR_RED,   NbMax.Cur.Value++;
      CEL_PrintKhz (FreqData[i].Cur.Cell, Couleur, FreqData[i].Cur.Value);

      Couleur = COLOR_DEFAULT;
      if      (FreqData[i].Min.Value == ColMin.Min.Value) Couleur=COLOR_GREEN, NbMin.Min.Value++;
      else if (FreqData[i].Min.Value == ColMin.Max.Value) Couleur=COLOR_RED,   NbMax.Min.Value++;
      CEL_PrintKhz (FreqData[i].Min.Cell, Couleur, FreqData[i].Min.Value);

      Couleur = COLOR_DEFAULT;
      if      (FreqData[i].Max.Value == ColMax.Min.Value) Couleur=COLOR_GREEN, NbMin.Max.Value++;
      else if (FreqData[i].Max.Value == ColMax.Max.Value) Couleur=COLOR_RED,   NbMax.Max.Value++;
      CEL_PrintKhz (FreqData[i].Max.Cell, Couleur, FreqData[i].Max.Value);
    }
   
    Total.Cur.Value = Total.Min.Value = Total.Max.Value = 0;
    for (i=0; i<NbCore; i++)
    {
      Total.Cur.Value += FreqData[i].Cur.Value;
      Total.Min.Value += FreqData[i].Min.Value;
      Total.Max.Value += FreqData[i].Max.Value;
    }
    CEL_PrintKhz (Total.Cur.Cell, COLOR_DEFAULT, Total.Cur.Value);
    CEL_PrintKhz (Total.Min.Cell, COLOR_DEFAULT, Total.Min.Value);
    CEL_PrintKhz (Total.Max.Cell, COLOR_DEFAULT, Total.Max.Value);
    
    Moyenne.Cur.Value = Total.Cur.Value / NbCore;
    Moyenne.Min.Value = Total.Min.Value / NbCore;
    Moyenne.Max.Value = Total.Max.Value / NbCore;
    CEL_PrintKhz (Moyenne.Cur.Cell, COLOR_DEFAULT, Moyenne.Cur.Value);
    CEL_PrintKhz (Moyenne.Min.Cell, COLOR_DEFAULT, Moyenne.Min.Value);
    CEL_PrintKhz (Moyenne.Max.Cell, COLOR_DEFAULT, Moyenne.Max.Value);

    CEL_PrintValColor (NbMin.Cur.Cell, NbMin.Cur.Value, COLOR_GREEN);
    CEL_PrintValColor (NbMin.Min.Cell, NbMin.Min.Value, COLOR_GREEN);
    CEL_PrintValColor (NbMin.Max.Cell, NbMin.Max.Value, COLOR_GREEN);

    NbInt.Cur.Value = NbCore - NbMin.Cur.Value - NbMax.Cur.Value;
    //NbInt.Min.Value = NbCore - NbMin.Min.Value - NbMax.Min.Value;
    //NbInt.Max.Value = NbCore - NbMin.Max.Value - NbMax.Max.Value;

    CEL_PrintValColor (NbInt.Cur.Cell, NbInt.Cur.Value, COLOR_ORANGE);
    //CEL_PrintValColor (NbInt.Min.Cell, NbInt.Min.Value, COLOR_ORANGE);
    //CEL_PrintValColor (NbInt.Max.Cell, NbInt.Max.Value, COLOR_ORANGE);

    CEL_PrintValColor (NbMax.Cur.Cell, NbMax.Cur.Value, COLOR_RED);
    CEL_PrintValColor (NbMax.Min.Cell, NbMax.Min.Value, COLOR_RED);
    CEL_PrintValColor (NbMax.Max.Cell, NbMax.Max.Value, COLOR_RED);

    CEL_PrintKhz (ExtrMin.Cur.Cell, COLOR_GREEN, ColCur.Min.Value);
    CEL_PrintKhz (ExtrMin.Min.Cell, COLOR_GREEN, ColMin.Min.Value);
    CEL_PrintKhz (ExtrMin.Max.Cell, COLOR_GREEN, ColMax.Min.Value);

    CEL_PrintKhz (ExtrMax.Cur.Cell, COLOR_RED, ColCur.Max.Value);
    CEL_PrintKhz (ExtrMax.Min.Cell, COLOR_RED, ColMin.Max.Value);
    CEL_PrintKhz (ExtrMax.Max.Cell, COLOR_RED, ColMax.Max.Value);

    DSP_Refresh (&Display, NbCore+12);
    sleep (1);
  }
  return 0;
}
