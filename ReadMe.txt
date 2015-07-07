Converts TGA textures to Operation Flashpoint PAA format.

New in 1.1:
-----------
- Fixed buffer overrun in compression code which sometimes caused crashes or produced invalid texture files
- Added option to mark texture file so that OFP does not try to interpolate alpha channel for DXT textures
- Compression for non-DXT textures is still too slow, optimized version has too many bugs.. maybe in 1.2
- Sharpening filters are only applied to mipmap textures

PAATool (C) 2004 feersum.endjinn@dnainternet.net

Contains portions of NVIDIA DXT library, (C) 2003 NVIDIA Corporation

This software is provided "as is", no warranty is given. Feel free to use provided source code as you want.

Compression of anything else than DXTC textures is slow. It is just bad code, source
code is provided so you can improve on it =) Also code won't work properly when compiled as "Debug" release, it's got something to do with local heaps and callbacks from nVidia's library not liking each other. I'm not MFC guru and it seems to work when built in "Release" mode, that's enough for me.