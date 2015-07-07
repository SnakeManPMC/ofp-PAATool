# ofp-PAATool

Operation Flashpoint PAATool by feersum.endjinn

Converts TGA textures to Operation Flashpoint PAA format.

New in 1.1:
- Fixed buffer overrun in compression code which sometimes caused crashes or produced invalid texture files
- Added option to mark texture file so that OFP does not try to interpolate alpha channel for DXT textures
- Compression for non-DXT textures is still too slow, optimized version has too many bugs.. maybe in 1.2
- Sharpening filters are only applied to mipmap textures

PAATool (C) 2004 feersum.endjinn
