# Houdini Plugins

## Overview
This is a place to put all of the Houdini plugins, scripts, etc… That I work on, so they don't get lost, and in the off chance they prove useful, others can find them.

## Config
###Node Color Palette###

Place this file in your $HOME/houdiniXX.YY/config directory, to get a custom palette. 

The included example looks like this:

![pallete](https://dl.dropboxusercontent.com/u/2141398/screenshot-area-2015-11-25-191702.png)

## Panels
###Snippets Panel###

Provides artists with a simple snippets manager, to save/use VEX and Python code. Pretty basic for now...

![snippets ui](https://dl.dropboxusercontent.com/u/2141398/snippet_v01_panel.png)

## Plugins
Consider these obsolete, but were good learning exercises at the time.

### POPs
* ~~**Repulse POP** - Creates a small field that helps to keep particles separated, based on their radius.~~
* **Bullet POP** - Uses the Bullet library to allow for RBD collisions. Only supports spheres right now, and is missing robust rotation support.

### SOPs
* **Pack SOP** - Uses the Bullet Library to pack a series of objects together.

## Pipeline
* **floo** - A semi-hacky way to manage jobs on a few computers.
* **mkmov** - Simple Python script for converting images to mp4 using ffmpeg.
 
## Scripts

* **mkmov** - Simple ffmpeg wrapper to bake movies, since ffmpeg is pretty old (or secretly avconv) on Debian.
* **htool.py** - Utility for setting Houdini versions in a given shell; called by bash function.
