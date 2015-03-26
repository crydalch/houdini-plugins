#!/usr/bin/python

# wrapper around ffmpeg command to make it easy to bake out decent mp4 images for vimeo and such

import sys
from subprocess import call

src_img = sys.argv[1]
dest_vid = sys.argv[2]

call(['~/bin/ffmpeg-2.5.4-64bit-static/ffmpeg -r 24 -i %s -b:v 20000k -vcodec libx264 -vf "curves=preset=lighter" -pix_fmt yuv420p %s' %(src_img,dest_vid)],shell=True)
