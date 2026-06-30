#include "ff.h"

#if _USE_LFN != 0
WCHAR ff_convert(WCHAR chr, UINT dir)
{
  (void)dir;
  return chr;
}

WCHAR ff_wtoupper(WCHAR chr)
{
  if ((chr >= (WCHAR)'a') && (chr <= (WCHAR)'z'))
  {
    chr = (WCHAR)(chr - ((WCHAR)'a' - (WCHAR)'A'));
  }
  return chr;
}
#endif
