# Houdini Plugins

## Overview
This is a place to put all of the Houdini plugins, scripts, etc… That I work on, so they don't get lost, and in the off chance they prove useful, others can find them.

Items in **bold** are current/ip, and _italicized_ are planned/ideas, regardless of whether or not there is code in the repo for said item.

### POPs
* **Repulse POP** - Creates a small field that helps to keep particles separated, based on their radius.
* **Bullet POP** - Uses the Bullet library to allow for RBD collisions. Only supports spheres right now, and is missing robust rotation support.

### SOPs
* **Pack SOP** - Uses the Bullet Library to pack a series of objects together.
* _Bullet SOP_ - Takes a series of points, simulates for the hip file's frame range, and caches the resulting points
    * This might need to replace the Bullet DOP, if that solver can't scale up to hundreds of thousands of bodies.

### DOPs
* **Bullet DOP** - Need to optimize the solver somehow; so it can solve in a stable, relatively fast way. On the order of millions of jelly beans.
* _Gas Shaped Vorticle Microsolver_ - Create non-spherical, artistic vorticles, to get custom shapes in vorticles. Think _The Emperor's New Groove_ style smoke.

### VEX - Functions
* _SumTraveledDepths()_ - Function that measures the total distance traveled inside of any objects found between the near/far distances.

### VEX - Shaders
* **Yummy** - Surface shader for hard, shiny, semi-transluscent objects.
* _Gummy_ - Light shader for fast, approximate SSS. Based on Pixar's SIGGRAPH talk _Anyone Can Cook_.

_(I wonder if Github would be willing to add support for *.vfl shaders in syntax highlighting…)_


### Desktop/Shelf
* _Shader Dev_ - A desktop for doing VEX shader development. Also has a shelf, which initially will only have a single Python tool, which recompiles the given .vfl into an OTL, and reloads that OTL into the current scene.

### Pipeline