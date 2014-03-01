junos-image-tools
=================

Tools for splitting and decompressing Juniper JUNOS software images.

JUNOS software images for specific platforms are distributed as combined images
and jboot files. One tool is for splitting out the kernel image from the
cd image that contains the software loaded later on. The other tool is for 
decompressing the cd images that are used by JUNOS. The format is little more 
than a stream of compressed chunks with a header to allow for quick seeking.