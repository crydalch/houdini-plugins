#!/bin/csh -f

set INCLUDEBULLET  = /usr/local/include/bullet
set BULLET_LIB = /usr/local/lib

hcustom -I$INCLUDEBULLET -L$BULLET_LIB -lBulletDynamics -lBulletCollision -lLinearMath POP_Bullet.C
