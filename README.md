This project displays 6 different objects in OpenGL. Five of them are the OSU shapes (sphere, cube, cylinder, cone, torus), and the sixth object is my Salmon OBJ model, reused from earlier.

What I Did

1. Created a struct to organize objects. I made a struct object that stores:
- name of the object
- the BMP texture file
- its display list ID
- the keyboard key used to select it
- the texture object

Then I created an array of 6 of these objects. This allowed me to build each object in a loop instead of repeating code.

2. Added texture loading. I wrote a helper function InitTextures() that:
- opens each BMP file
- loads it using BmpToTexture
- generates a texture (GLuint)
- sets texture wrapping + filtering
- uploads it with glTexImage2D

Each object has its own BMP texture.

3. Built display lists. In InitLists() I loop through the object array and:
- bind the object’s texture
- set a neutral material
- use a switch statement to draw the correct OSU shape
- for the OBJ object (the salmon), I scaled it and called its list

This makes all objects ready to be drawn quickly.

4. Keyboard controls. I added:
- keys 0–5 → switch between objects
- key t → toggle texture on/off
This lets the user flip through the 6 objects easily.

5. Moving light source
   
The light moves in a circle around the scene using the Time variable. I use SetPointLight or SetSpotLight depending on the user’s input.

Main Features
Six objects (5 OSU + 1 OBJ)
One texture per object
Texture toggle (on/off)
Moving animated light
Struct-based object system
Display lists for efficient rendering
Simple keyboard interaction
