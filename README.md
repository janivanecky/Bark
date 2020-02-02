# Bark

This program generates a tree bark using mass spring system. Tree bark is represented by nodes that are connected by springs. Nodes are organized into layers, each layer is a surface of a cylinder with successive layers having larger radius the previous ones. This creates sort of a tree trunk.
Nodes are connected within layer and between layers.
Spring breaks when it's stretched more than it can take (strength of each connection is generated randomly at the start of the simulation using perlin noise).
During simulation, each node is moved away from the center of cylinder in the horizontal plane, which causes tension on the springs.

## Modes

Modes are switch using #define CYLINDER in main.cpp. Obviously if defined, Cylinder mode is used, otherwise Flat mode is used.

### Cylinder

* bark is visualised in 3D 

![alt text](https://github.com/janivanecky/Bark/blob/master/Bark/img/bark1.png "Cylinder mode")

### Flat

* only flat texture of the bark is shown
* exporting the resulting texture is available in this mode

![alt text](https://github.com/janivanecky/Bark/blob/master/Bark/img/flat1.png "Flat mode")

## Parameters for the system

* strengthSigma - variance in strengths of the connections
* strengthBase - constant that is added to the strength of each connection
* interLayerStiffness - influence of the connections from other layers on the force acting on the node
* intraLayerStiffness - influence of the connections within layer on the force acting on the node
* bounded - boolean value specifying whether connection in the innermost layer are able to break
* baseFreq - specifies lowest frequency used by Perlin noise for generating strengths
* damp - strength of the spring damping
* THICKNESS - number of layers (#define in a main.cpp)

## Controls

* W - resets the simulation
* B - in Flat mode, creates and saves the texture as a binary file in RGBA32 format.
* S - in Flat mode, saves the texture in the DDS format

## Notes

[DXTK](https://directxtk.codeplex.com/) was used to save the texture to the DDS files.

