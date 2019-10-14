/*
  John M. Gaspar (jsh58@wildcats.unh.edu)
  April 2015 (updated 2016, 2017)

  Header file for NGmerge.c.
*/
#define VERSION     "0.2_dev"

// constants
#define MAX_SIZE    1024    // maximum length of input lines (incl. seq/qual)
#define NOTMATCH    1.5f    // stitch failure
#define COM         ", "    // separator for input file names
#define CSV         ",\t"   // separator for quality score profile
#define NA          "NA"    // n/a (for output log file)

// default parameter values
#define DEFOVER     20      // min. overlap
#define DEFDOVE     50      // min. overlap for dovetailed alignments
#define DEFMISM     0.1f    // mismatch fraction
#define OFFSET      33      // fastq quality offset (Sanger = 33)
#define MAXQUAL     40      // maximum quality score (0-based)
#define DEFTHR      1       // number of threads

// fastq parts
enum fastq { HEAD, SEQ, PLUS, QUAL, FASTQ };  // lines of a fastq read
#define BEGIN       '@'     // beginning of header line
#define PLUSCHAR    '+'     // beginning of 3rd line
#define EXTRA       2       // save 2 extra strings for 2nd read:
                            //   revComp(seq) and rev(qual)

// command-line options
#define OPTIONS     "h1:2:o:f:l:m:p:de:c:saj:bzyigq:u:w:t:n:vV"
#define HELP        'h'
#define FIRST       '1'
#define SECOND      '2'
#define OUTFILE     'o'
#define UNFILE      'f'
#define LOGFILE     'l'
#define OVERLAP     'm'
#define MISMATCH    'p'
#define DOVEOPT     'd'
#define DOVEOVER    'e'
#define DOVEFILE    'c'
#define MAXOPT      's'
#define ADAPTOPT    'a'
#define ALNFILE     'j'
#define DIFFOPT     'b'
#define GZOPT       'z'
#define UNGZOPT     'y'
#define INTEROPT    'i'
#define FJOINOPT    'g'
#define QUALITY     'q'
#define SETQUAL     'u'
#define QUALFILE    'w'
#define DELIM       't'
#define THREADS     'n'
#define VERBOSE     'v'
#define VERSOPT     'V'

static struct option long_options[] = {
  {"help", no_argument, NULL, HELP},
  {"verbose", no_argument, NULL, VERBOSE},
  {"version", no_argument, NULL, VERSOPT},
  {0, 0, 0, 0}
};

// extensions for output files
#define ONEEXT      "_1.fastq"
#define TWOEXT      "_2.fastq"
#define GZEXT       ".gz"   // for gzip compression

// OMP locks
enum omp_locks { OUT, UN, LOG, DOVE, ALN, OMP_LOCKS };

// error messages
enum errCode { ERRFILE, ERROPEN, ERROPENW, ERRCLOSE, ERRUNK,
  ERRMEM, ERRSEQ, ERRQUAL, ERRHEAD, ERRINT, ERRFLOAT, ERRPARAM,
  ERROVER, ERRMISM, ERRFASTQ, ERROFFSET, ERRUNGET, ERRGZIP,
  ERRTHREAD, ERRNAME, ERRRANGE, ERRDEFQ, DEFERR
};
const char* errMsg[] = { "Need input/output files",
  ": cannot open file for reading",
  ": cannot open file for writing",
  ": cannot close file",
  ": unknown nucleotide",
  "Cannot allocate memory",
  "Cannot load sequence",
  "Sequence/quality scores do not match",
  ": not matched in input files",
  ": cannot convert to int",
  ": cannot convert to float",
  ": unknown command-line argument",
  "Overlap must be greater than 0",
  "Mismatch must be in [0,1)",
  "Input file does not follow fastq format",
  ": quality score outside of set range",
  "Failure in ungetc() call",
  "Cannot pipe in gzip compressed file (use zcat instead)",
  "Number of threads must be >= 1",
  ": output filename cannot start with '-'",
  ": file missing values for quality score range",
  "Cannot increase max. quality score with default error profile",
  "Unknown error"
};

// generic File type
typedef union file {
  FILE* f;
  gzFile gzf;
} File;

// error profiles -- matches and mismatches
const char match_profile[ MAXQUAL + 1 ][ MAXQUAL + 1 ] = {
  {25, 25, 25, 25, 25, 25, 25, 25, 26, 26, 26, 27, 27, 28, 28, 29, 29, 30, 30, 31, 32, 32, 33, 33, 34, 34, 35, 35, 36, 36, 37, 37, 37, 38, 38, 39, 39, 39, 40, 40, 40},
  {25, 25, 25, 25, 25, 25, 25, 25, 26, 26, 26, 27, 27, 28, 28, 29, 29, 30, 30, 31, 32, 32, 33, 33, 34, 34, 35, 35, 36, 36, 37, 37, 37, 38, 38, 39, 39, 39, 40, 40, 40},
  {25, 25, 25, 25, 25, 25, 25, 25, 26, 26, 26, 27, 27, 28, 28, 29, 29, 30, 30, 31, 32, 32, 33, 33, 34, 34, 35, 35, 36, 36, 37, 37, 37, 38, 38, 39, 39, 39, 40, 40, 40},
  {25, 25, 25, 25, 25, 25, 25, 25, 25, 26, 26, 26, 27, 27, 28, 28, 29, 29, 30, 30, 31, 32, 32, 33, 33, 34, 34, 35, 35, 36, 36, 37, 37, 37, 38, 38, 39, 39, 39, 40, 40},
  {25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 26, 26, 27, 27, 28, 28, 29, 29, 30, 31, 31, 32, 32, 33, 33, 34, 34, 35, 35, 36, 36, 37, 37, 37, 38, 38, 39, 39, 39, 40},
  {25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 26, 26, 27, 27, 28, 28, 29, 30, 30, 31, 31, 32, 33, 33, 34, 34, 35, 35, 35, 36, 36, 37, 37, 38, 38, 38, 39, 39, 40},
  {25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 26, 26, 26, 27, 27, 28, 29, 29, 30, 30, 31, 32, 32, 33, 33, 34, 34, 35, 35, 36, 36, 36, 37, 37, 38, 38, 38, 39, 39},
  {25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 26, 26, 27, 27, 28, 28, 29, 30, 30, 31, 31, 32, 33, 33, 34, 34, 34, 35, 35, 36, 36, 37, 37, 37, 38, 38, 39, 39},
  {25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 26, 26, 27, 27, 28, 28, 29, 30, 30, 31, 31, 32, 32, 33, 33, 34, 34, 35, 35, 36, 36, 36, 37, 37, 38, 38, 39, 39},
  {25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 26, 26, 27, 27, 28, 28, 29, 29, 30, 31, 31, 32, 32, 33, 33, 34, 34, 35, 35, 35, 36, 36, 37, 37, 38, 38, 39, 39},
  {26, 26, 26, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 26, 26, 27, 27, 28, 28, 29, 29, 30, 31, 31, 32, 32, 33, 33, 34, 34, 34, 35, 35, 36, 36, 37, 37, 38, 38, 39, 39},
  {26, 26, 26, 26, 26, 25, 25, 25, 25, 25, 25, 25, 26, 26, 26, 27, 27, 28, 28, 29, 30, 30, 31, 31, 32, 32, 33, 33, 33, 34, 34, 35, 35, 36, 36, 37, 37, 38, 38, 39, 39},
  {27, 27, 27, 26, 26, 26, 26, 25, 25, 25, 26, 26, 26, 26, 27, 27, 27, 28, 29, 29, 30, 30, 31, 31, 32, 32, 33, 33, 33, 34, 34, 35, 35, 36, 36, 36, 37, 37, 38, 39, 39},
  {27, 27, 27, 27, 26, 26, 26, 26, 26, 26, 26, 26, 26, 27, 27, 27, 28, 28, 29, 29, 30, 30, 31, 32, 32, 32, 33, 33, 34, 34, 34, 35, 35, 36, 36, 36, 37, 37, 38, 39, 39},
  {28, 28, 28, 27, 27, 27, 27, 26, 26, 26, 26, 26, 27, 27, 27, 28, 28, 29, 29, 30, 30, 31, 31, 32, 32, 33, 33, 33, 34, 34, 34, 35, 35, 36, 36, 36, 37, 37, 38, 39, 39},
  {29, 29, 29, 28, 28, 27, 27, 27, 27, 27, 27, 27, 27, 27, 28, 28, 28, 29, 29, 30, 30, 31, 31, 32, 32, 33, 33, 33, 34, 34, 35, 35, 35, 36, 36, 37, 37, 37, 38, 39, 39},
  {29, 29, 29, 29, 28, 28, 28, 28, 27, 27, 27, 28, 28, 28, 28, 29, 29, 29, 30, 30, 31, 31, 32, 32, 33, 33, 33, 34, 34, 34, 35, 35, 35, 36, 36, 37, 37, 38, 38, 39, 39},
  {30, 30, 30, 29, 29, 29, 28, 28, 28, 28, 28, 28, 28, 28, 29, 29, 29, 30, 30, 31, 31, 32, 32, 33, 33, 33, 34, 34, 34, 35, 35, 35, 36, 36, 36, 37, 37, 38, 38, 39, 39},
  {30, 30, 30, 30, 30, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 30, 30, 31, 31, 32, 32, 33, 33, 33, 34, 34, 34, 35, 35, 35, 35, 36, 36, 36, 37, 37, 38, 38, 39, 39},
  {31, 31, 31, 31, 30, 30, 30, 29, 29, 29, 29, 29, 29, 30, 30, 30, 30, 31, 31, 32, 32, 33, 33, 33, 34, 34, 34, 35, 35, 35, 35, 36, 36, 36, 37, 37, 37, 38, 38, 39, 39},
  {32, 32, 32, 31, 31, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 31, 31, 32, 32, 33, 33, 34, 34, 34, 35, 35, 35, 35, 35, 36, 36, 36, 36, 37, 37, 37, 38, 38, 39, 39},
  {32, 32, 32, 32, 31, 31, 31, 31, 30, 30, 30, 30, 30, 31, 31, 31, 31, 32, 32, 33, 33, 34, 34, 34, 35, 35, 35, 35, 36, 36, 36, 36, 36, 37, 37, 37, 38, 38, 38, 39, 40},
  {33, 33, 33, 32, 32, 32, 31, 31, 31, 31, 31, 31, 31, 31, 31, 32, 32, 32, 33, 33, 34, 34, 35, 35, 35, 36, 36, 36, 36, 36, 36, 36, 37, 37, 37, 37, 38, 38, 39, 39, 40},
  {34, 34, 34, 33, 33, 32, 32, 32, 32, 32, 31, 31, 32, 32, 32, 32, 32, 33, 33, 33, 34, 34, 35, 35, 36, 36, 36, 36, 36, 36, 37, 37, 37, 37, 37, 38, 38, 38, 39, 39, 40},
  {34, 34, 34, 34, 33, 33, 33, 32, 32, 32, 32, 32, 32, 32, 32, 33, 33, 33, 33, 34, 34, 35, 35, 36, 36, 36, 37, 37, 37, 37, 37, 37, 37, 37, 37, 38, 38, 38, 39, 39, 40},
  {35, 35, 35, 34, 34, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 34, 34, 34, 35, 35, 36, 36, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 38, 38, 38, 39, 39, 39, 40},
  {35, 35, 35, 35, 34, 34, 34, 34, 33, 33, 33, 33, 33, 33, 33, 34, 34, 34, 34, 35, 35, 36, 36, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 38, 38, 38, 38, 39, 39, 40, 40},
  {36, 36, 36, 35, 35, 35, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 35, 35, 36, 36, 36, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 38, 38, 38, 38, 39, 39, 40, 40},
  {36, 36, 36, 36, 35, 35, 35, 35, 34, 34, 34, 34, 34, 34, 34, 34, 35, 35, 35, 36, 36, 36, 37, 37, 37, 37, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 39, 39, 39, 40, 40},
  {37, 37, 37, 36, 36, 36, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 36, 36, 36, 37, 37, 37, 37, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 39, 39, 39, 40, 40},
  {37, 37, 37, 37, 36, 36, 36, 36, 35, 35, 35, 35, 35, 35, 35, 35, 35, 36, 36, 36, 36, 37, 37, 37, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 39, 39, 39, 40, 40},
  {38, 38, 38, 37, 37, 37, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 37, 37, 37, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 39, 39, 39, 39, 40, 40},
  {38, 38, 38, 38, 37, 37, 37, 37, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 37, 37, 37, 37, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 39, 39, 39, 39, 40, 40},
  {39, 39, 39, 38, 38, 37, 37, 37, 37, 37, 37, 36, 36, 36, 36, 37, 37, 37, 37, 37, 37, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 39, 39, 39, 39, 39, 39, 40, 40},
  {39, 39, 39, 39, 38, 38, 38, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 38, 38, 38, 38, 38, 38, 38, 38, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 40, 40},
  {39, 39, 39, 39, 39, 38, 38, 38, 38, 38, 38, 37, 37, 37, 37, 37, 37, 37, 38, 38, 38, 38, 38, 38, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 40, 40},
  {40, 40, 40, 39, 39, 39, 39, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 40, 40},
  {40, 40, 40, 40, 39, 39, 39, 39, 39, 39, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 40, 40},
  {40, 40, 40, 40, 40, 40, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 40, 40, 40},
  {40, 40, 40, 40, 40, 40, 40, 40, 40, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40},
  {40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40}
};

const char mismatch_profile[ MAXQUAL + 1 ][ MAXQUAL + 1 ] = {
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 7, 10, 12, 13, 15, 16, 18, 19, 20, 21, 23, 25, 27, 29, 31, 33, 35, 37, 39},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 7, 10, 12, 13, 15, 16, 18, 19, 20, 21, 23, 25, 27, 29, 31, 33, 35, 37, 39},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 7, 10, 12, 13, 15, 16, 18, 19, 20, 21, 23, 25, 27, 29, 31, 33, 35, 37, 39},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 6, 9, 11, 13, 14, 16, 17, 18, 20, 21, 22, 24, 26, 29, 31, 33, 35, 37, 38},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 5, 8, 11, 12, 14, 15, 17, 18, 19, 20, 22, 23, 25, 28, 30, 32, 34, 36, 38},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 4, 8, 10, 12, 13, 15, 16, 17, 19, 20, 21, 23, 25, 27, 29, 31, 33, 35, 37},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 3, 7, 9, 11, 13, 14, 16, 17, 18, 19, 21, 22, 24, 26, 28, 31, 33, 35, 37},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 6, 9, 11, 12, 14, 15, 16, 18, 19, 20, 21, 23, 25, 27, 30, 32, 34, 36},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 2, 2, 5, 8, 10, 12, 13, 14, 16, 17, 18, 19, 21, 22, 24, 26, 29, 31, 34, 36},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 2, 2, 4, 7, 9, 11, 12, 14, 15, 16, 18, 19, 20, 22, 23, 25, 28, 31, 33, 35},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 2, 2, 2, 6, 8, 10, 12, 13, 15, 16, 17, 18, 20, 21, 23, 25, 27, 30, 32, 34},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 5, 8, 10, 11, 13, 14, 15, 17, 18, 19, 20, 22, 24, 26, 29, 31, 34},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 4, 7, 9, 11, 12, 13, 15, 16, 17, 18, 20, 21, 23, 25, 28, 30, 33},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 3, 6, 8, 10, 11, 13, 14, 15, 17, 18, 19, 21, 22, 24, 27, 30, 32},
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 5, 7, 9, 11, 12, 13, 15, 16, 17, 18, 20, 21, 23, 26, 28, 31},
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 3, 6, 8, 10, 11, 13, 14, 15, 17, 18, 19, 21, 22, 24, 27, 30},
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 5, 7, 9, 10, 12, 13, 14, 16, 17, 18, 20, 21, 23, 26, 29},
  {2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 5, 7, 9, 11, 12, 13, 15, 16, 18, 19, 20, 22, 24, 27},
  {2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 4, 7, 9, 11, 12, 14, 15, 17, 18, 20, 21, 23, 26},
  {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 3, 7, 9, 11, 12, 14, 16, 17, 19, 20, 22, 25},
  {7, 7, 7, 6, 5, 4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 3, 7, 9, 11, 13, 15, 16, 18, 19, 21, 23},
  {9, 9, 9, 8, 8, 7, 6, 6, 4, 3, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 4, 7, 10, 12, 14, 15, 17, 19, 20, 22},
  {11, 11, 11, 10, 10, 9, 9, 8, 7, 6, 6, 4, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 5, 8, 10, 13, 14, 16, 18, 19, 21},
  {12, 12, 12, 12, 12, 11, 10, 10, 9, 9, 8, 7, 6, 5, 3, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 7, 9, 11, 13, 15, 17, 19, 20},
  {14, 14, 14, 14, 13, 13, 12, 12, 11, 10, 10, 9, 8, 7, 6, 5, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 4, 8, 10, 12, 14, 16, 18, 20},
  {15, 15, 15, 15, 14, 14, 14, 13, 13, 12, 12, 11, 10, 9, 9, 8, 6, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 6, 9, 11, 13, 15, 17, 19},
  {17, 17, 17, 16, 16, 15, 15, 15, 14, 14, 13, 13, 12, 11, 11, 10, 8, 6, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 4, 8, 10, 13, 14, 16, 18},
  {18, 18, 18, 18, 17, 17, 16, 16, 15, 15, 14, 14, 13, 13, 12, 11, 10, 8, 6, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 3, 7, 10, 12, 14, 15, 17},
  {19, 19, 19, 19, 18, 18, 18, 17, 17, 16, 16, 15, 15, 14, 14, 13, 12, 10, 8, 5, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 7, 9, 11, 13, 14, 16},
  {20, 20, 20, 20, 20, 19, 19, 18, 18, 17, 17, 16, 16, 15, 15, 14, 13, 12, 10, 8, 5, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 6, 8, 10, 12, 14, 15},
  {22, 22, 22, 21, 21, 20, 20, 20, 19, 19, 18, 18, 17, 17, 16, 16, 15, 13, 12, 10, 8, 6, 2, 2, 2, 2, 2, 1, 1, 2, 2, 2, 2, 2, 3, 6, 8, 10, 11, 13, 14},
  {23, 23, 23, 23, 22, 22, 21, 21, 20, 20, 19, 19, 18, 18, 17, 17, 16, 15, 14, 12, 11, 9, 7, 4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 4, 6, 8, 9, 11, 12, 13},
  {25, 25, 25, 25, 24, 24, 23, 22, 22, 21, 21, 20, 19, 19, 19, 18, 17, 16, 15, 14, 13, 11, 10, 8, 7, 6, 4, 2, 2, 2, 2, 2, 2, 3, 5, 6, 7, 9, 10, 11, 13},
  {28, 28, 28, 27, 26, 25, 25, 24, 23, 23, 22, 21, 21, 20, 20, 19, 19, 18, 17, 16, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 5, 4, 4, 5, 6, 6, 7, 9, 10, 11, 12},
  {30, 30, 30, 29, 28, 28, 27, 26, 25, 25, 24, 23, 22, 22, 21, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 12, 11, 10, 9, 8, 8, 7, 7, 7, 7, 7, 8, 9, 10, 11, 12},
  {32, 32, 32, 31, 30, 30, 29, 29, 28, 27, 26, 25, 24, 24, 23, 22, 21, 21, 20, 19, 18, 17, 16, 15, 15, 14, 13, 12, 12, 11, 10, 10, 9, 9, 9, 9, 9, 9, 10, 11, 11},
  {33, 33, 33, 33, 32, 32, 31, 31, 30, 29, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 20, 19, 18, 17, 17, 16, 15, 15, 14, 13, 13, 12, 11, 11, 10, 10, 10, 10, 10, 11, 11},
  {35, 35, 35, 35, 34, 34, 33, 33, 32, 32, 31, 30, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 18, 17, 17, 16, 15, 15, 14, 13, 13, 12, 12, 11, 11, 11, 11, 11},
  {37, 37, 37, 36, 36, 36, 35, 35, 34, 34, 33, 33, 32, 31, 31, 30, 29, 28, 26, 25, 24, 23, 22, 21, 20, 20, 19, 18, 18, 17, 16, 16, 15, 14, 14, 13, 13, 12, 12, 11, 11},
  {39, 39, 39, 38, 38, 37, 37, 37, 36, 36, 35, 35, 34, 34, 33, 32, 32, 31, 30, 29, 27, 26, 25, 24, 23, 22, 21, 20, 20, 19, 18, 17, 17, 16, 15, 15, 14, 13, 13, 12, 11},
  {40, 40, 40, 40, 40, 39, 39, 39, 38, 38, 37, 37, 36, 36, 35, 35, 34, 33, 33, 32, 31, 30, 29, 28, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 17, 16, 15, 14, 13, 13, 12}
};
