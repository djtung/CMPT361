//---------------------------------------------------------------------------------------------------------
/*
CMPT 361 Assignment 1 - FruitTetris implementation Sample Skeleton Code

- This is ONLY a skeleton code showing:
How to use multiple buffers to store different objects
An efficient scheme to represent the grids and blocks

- Compile and Run:
Type make in terminal, then type ./FruitTetris

This code is extracted from Connor MacLeod's (crmacleo@sfu.ca) assignment submission
by Rui Ma (ruim@sfu.ca) on 2014-03-04. 

Modified in Sep 2014 by Honghua Li (honghual@sfu.ca).
*/

#include "include/Angel.h"
#include <cstdlib>
#include <iostream>

using namespace std;

typedef Angel::vec4 point4;
typedef Angel::vec4 color4;

const int NumVertices = 36; //(6 faces)(2 triangles/face)(3 vertices/triangle)

point4 points[NumVertices];
color4 colors[NumVertices];

point4 vertices[8] = {
    point4( -5.0, -5.0,  5.0, 10.0 ),
    point4( -5.0,  5.0,  5.0, 10.0 ),
    point4(  5.0,  5.0,  5.0, 10.0 ),
    point4(  5.0, -5.0,  5.0, 10.0 ),
    point4( -5.0, -5.0, -5.0, 10.0 ),
    point4( -5.0,  5.0, -5.0, 10.0 ),
    point4(  5.0,  5.0, -5.0, 10.0 ),
    point4(  5.0, -5.0, -5.0, 10.0 )
};

// xsize and ysize represent the window size - updated if window is reshaped to prevent stretching of the game
GLfloat xsize = 400; 
GLfloat ysize = 720;

// current tile
vec2 tile[4]; // An array of 4 2d vectors representing displacement from a 'center' piece of the tile, on the grid
vec2 tilepos = vec2(5, 19); // The position of the current tile using grid coordinates ((0,0) is the bttm lft corner)

// An array storing all possible orientations of all possible tiles
// The 'tile' array will always be some element [i][j] of this array (an array of vec2)
vec2 allRotationsIshape[4][4] = 
	{{vec2(-2, 0), vec2(-1,0), vec2(0, 0), vec2(1,0)},
	{vec2(0, -2), vec2(0, -1), vec2(0, 0), vec2(0, 1)},
	{vec2(-2, 0), vec2(-1,0), vec2(0, 0), vec2(1,0)},
	{vec2(0, -2), vec2(0, -1), vec2(0, 0), vec2(0, 1)}};

vec2 allRotationsSshape[4][4] = 
	{{vec2(-1, -1), vec2(0,-1), vec2(0, 0), vec2(1,0)},
	{vec2(1, -1), vec2(1, 0), vec2(0, 0), vec2(0, 1)},
	{vec2(-1, -1), vec2(0,-1), vec2(0, 0), vec2(1,0)},
	{vec2(1, -1), vec2(1, 0), vec2(0, 0), vec2(0, 1)}};

vec2 allRotationsLshape[4][4] = 
	{{vec2(-1, -1), vec2(-1,0), vec2(0, 0), vec2(1, 0)},
	{vec2(1, -1), vec2(0, -1), vec2(0,0), vec2(0, 1)},     
	{vec2(1, 1), vec2(1, 0), vec2(0, 0), vec2(-1,  0)},  
	{vec2(-1,1), vec2(0, 1), vec2(0, 0), vec2(0, -1)}};

// shape I = 0
// shape S = 1
// shape L = 2
int tiletype;
int tileorient;

color4 vertex_colors[8] = {
    color4( 0.0, 0.0, 0.0, 1.0 ),  // black
    color4( 1.0, 0.0, 0.0, 1.0 ),  // red
    color4( 1.0, 1.0, 0.0, 1.0 ),  // yellow
    color4( 0.0, 1.0, 0.0, 1.0 ),  // green
    color4( 0.0, 0.0, 1.0, 1.0 ),  // blue
    color4( 1.0, 0.0, 1.0, 1.0 ),  // magenta
    color4( 1.0, 1.0, 1.0, 1.0 ),  // white
    color4( 0.0, 1.0, 1.0, 1.0 )   // cyan
};

// colors
vec4 white  = vec4(1.0, 1.0, 1.0, 1.0);
vec4 black  = vec4(0.0, 0.0, 0.0, 1.0); // 5 or any (default color)

vec4 purple = vec4(1.0, 0.0, 1.0, 1.0); // 0
vec4 red = vec4(1.0, 0.0, 0.0, 1.0); // 1
vec4 yellow = vec4(1.0, 1.0, 0.0, 1.0); // 2
vec4 green = vec4(0.0, 1.0, 0.0, 1.0); // 3
vec4 orange = vec4(1.0, 0.5, 0.0, 1.0); // 4
vec4 gray = vec4(0.75, 0.75, 0.75, 1.0);

//current tile colours
vec4 newcolours[144];

vec4 tilecolor;

 
//board[x][y] represents whether the cell (x,y) is occupied
bool board[10][20];

//An array containing the colour of each of the 10*20*2*3 vertices that make up the board
//Initially, all will be set to black. As tiles are placed, sets of 6 vertices (2 triangles; 1 square)
//will be set to the appropriate colour in this array before updating the corresponding VBO
vec4 boardcolours[1200];

// location of vertex attributes in the shader program
GLuint vPosition;
GLuint vColor;

// locations of uniform variables in shader program
GLuint locxsize;
GLuint locysize;

// VAO and VBO
GLuint vaoIDs[4]; // One VAO for each object: the grid, the board, the current piece, the robot arm
GLuint vboIDs[8]; // Two Vertex Buffer Objects for each VAO (specifying vertex positions and colours, respectively)

//VIEWING
// Viewing transformation parameters

GLfloat radius = 40.0;
GLfloat cameraangle = 0.0;
GLfloat phi = 0.0;

const GLfloat  dr = 5.0 * DegreesToRadians;

//GLuint  model_view;  // model-view matrix uniform shader variable location

// Projection transformation parameters

GLfloat  zNear = 0.1, zFar = 50;


// Array of rotation angles (in degrees) for each rotation axis
enum { Base = 0, LowerArm = 1, UpperArm = 2, NumAngles = 3 };
int      Axis = Base;
GLfloat  Theta[NumAngles] = { 0.0 };

mat4 mv;
GLuint  model_view, projection; // projection matrix uniform shader variable location

const GLfloat BASE_HEIGHT      = 2.0;
const GLfloat BASE_WIDTH       = 5.0;
const GLfloat LOWER_ARM_HEIGHT = 11.0;
const GLfloat LOWER_ARM_WIDTH  = 0.4;
const GLfloat UPPER_ARM_HEIGHT = 11.0;
const GLfloat UPPER_ARM_WIDTH  = 0.4;
const GLfloat BASE_OFFSET = -4.0;
//

//-------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------
int Index = 0;

void quad( int a, int b, int c, int d )
{
    colors[Index] = vertex_colors[7]; points[Index] = vertices[a]; Index++;
    colors[Index] = vertex_colors[7]; points[Index] = vertices[b]; Index++;
    colors[Index] = vertex_colors[7]; points[Index] = vertices[c]; Index++;
    colors[Index] = vertex_colors[7]; points[Index] = vertices[a]; Index++;
    colors[Index] = vertex_colors[7]; points[Index] = vertices[c]; Index++;
    colors[Index] = vertex_colors[7]; points[Index] = vertices[d]; Index++;
}

void colorcube()
{
    quad( 1, 0, 3, 2 );
    quad( 2, 3, 7, 6 );
    quad( 3, 0, 4, 7 );
    quad( 6, 5, 1, 2 );
    quad( 4, 5, 6, 7 );
    quad( 5, 4, 0, 1 );
}

//----------------------------------------------------------------------------
vec4 int_to_color(int x)
{
	if(x==0){
		return purple;
	}
	else if(x==1){
		return red;
	}
	else if(x==2){
		return yellow;
	}
	else if(x==3){
		return green;
	}
	else if(x==4){
		return orange;
	}
	else{
		return black;
	}
}

//-------------------------------------------------------------------------------------------------------------------
//gets and returns the position of the tip of the upper robot arm (for tile placement)
//					^
//					|
//					|
//					|
//					y
//					|
//					|
//					|
// <--------- x -----

GLfloat getx()
{
	GLfloat xpos = -BASE_OFFSET;
	//GLfloat ypos = BASE_HEIGHT;

	//ypos += (LOWER_ARM_HEIGHT * cos(Theta[LowerArm]*DegreesToRadians));
	xpos += (LOWER_ARM_HEIGHT * sin(Theta[LowerArm]*DegreesToRadians));

	//ypos += (UPPER_ARM_HEIGHT * cos((Theta[LowerArm]+Theta[UpperArm])*DegreesToRadians));
	xpos += (UPPER_ARM_HEIGHT * sin((Theta[LowerArm]+Theta[UpperArm])*DegreesToRadians));
	
	return -xpos;


}

GLfloat gety()
{
	//GLfloat xpos = -BASE_OFFSET;
	GLfloat ypos = BASE_HEIGHT;

	ypos += (LOWER_ARM_HEIGHT * cos(Theta[LowerArm]*DegreesToRadians));
	//xpos += (LOWER_ARM_HEIGHT * sin(Theta[LowerArm]*DegreesToRadians));

	ypos += (UPPER_ARM_HEIGHT * cos((Theta[LowerArm]+Theta[UpperArm])*DegreesToRadians));
	//xpos += (UPPER_ARM_HEIGHT * sin((Theta[LowerArm]+Theta[UpperArm])*DegreesToRadians));
	
	return ypos;
}

//-------------------------------------------------------------------------------------------------------------------
//check if the current tile shape and orientation can fit on the game board
bool canplace(){
	int newboardx; 
	int newboardy;

	if(tiletype == 0)
	{
		for (int i = 0; i < 4; i++) 
		{
			newboardx = round(getx()) + allRotationsIshape[tileorient][i].x;
			newboardy = round(gety()) + allRotationsIshape[tileorient][i].y;
			if(board[newboardx][newboardy] == 1 || newboardx < 0 || newboardx > 9 || newboardy < 0 || newboardy > 19)
			{
				return false;
			}
		}
	}
	if(tiletype == 1)
	{
		for (int i = 0; i < 4; i++) 
		{
			newboardx = round(getx()) + allRotationsSshape[tileorient][i].x;
			newboardy = round(gety()) + allRotationsSshape[tileorient][i].y;

			if(board[newboardx][newboardy] == 1 || newboardx < 0 || newboardx > 9 || newboardy < 0 || newboardy > 19)
			{
				return false;
			}
		}
	}
	if(tiletype == 2)
	{
		for (int i = 0; i < 4; i++) 
		{
			newboardx = round(getx()) + allRotationsLshape[tileorient][i].x;
			newboardy = round(gety()) + allRotationsLshape[tileorient][i].y;

			if(board[newboardx][newboardy] == 1 || newboardx < 0 || newboardx > 9 || newboardy < 0 || newboardy > 19)
			{
				return false;
			}
		}
	}
	return true;
}

//-------------------------------------------------------------------------------------------------------------------

// When the current tile is moved or rotated (or created), update the VBO containing its vertex position data
void updatetile()
{
	// Bind the VBO containing current tile vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[4]); 

	// For each of the 4 'cells' of the tile,
	for (int i = 0; i < 4; i++) 
	{
		// Calculate the grid coordinates of the cell
		tilepos.x = getx();
		tilepos.y = gety();

		GLfloat x = tilepos.x + tile[i].x; 
		GLfloat y = tilepos.y + tile[i].y;

		// Create the 4 corners of the square - these vertices are using location in pixels
		// These vertices are later converted by the vertex shader
		vec4 p1 = vec4((x * 1.0), (y * 1.0), -.5, 1);
		vec4 p2 = vec4((x * 1.0), 1.0 + (y * 1.0), -.5, 1);
		vec4 p3 = vec4(1.0 + (x * 1.0), (y * 1.0), -.5, 1);
		vec4 p4 = vec4(1.0 + (x * 1.0), 1.0 + (y * 1.0), -.5, 1);
		vec4 p5 = vec4((x * 1.0), (y * 1.0), .5, 1);
		vec4 p6 = vec4((x * 1.0), 1.0 + (y * 1.0), .5, 1);
		vec4 p7 = vec4(1.0 + (x * 1.0), (y * 1.0), .5, 1);
		vec4 p8 = vec4(1.0 + (x * 1.0), 1.0 + (y * 1.0), .5, 1);

		// Two points are used by two triangles each
		vec4 newpoints[36] = {p1, p2, p3, p2, p3, p4, 
							  p5, p6, p7, p6, p7, p8,
							  p3, p4, p8, p3, p7, p8,
							  p1, p2, p6, p1, p5, p6,
							  p4, p2, p6, p4, p8, p6,
							  p3, p1, p5, p3, p7, p5};


		// Put new data in the VBO
		glBufferSubData(GL_ARRAY_BUFFER, i*36*sizeof(vec4), 36*sizeof(vec4), newpoints); 
	}

	if(!canplace())
	{
		for (int i = 0; i < 144; i++) newcolours[i] = gray;
	}
	else if(canplace())
	{
		for (int i = 0; i < 144; i++) newcolours[i] = tilecolor;
	}

	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[5]); // Bind the VBO containing current tile vertex colours
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(newcolours), newcolours); // Put the colour data in the VBO
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}

//-------------------------------------------------------------------------------------------------------------------


//updates the board
void updateboard() {
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);
	glBufferData(GL_ARRAY_BUFFER, 1200*sizeof(vec4), boardcolours, GL_DYNAMIC_DRAW);
}

//-------------------------------------------------------------------------------------------------------------------
// Called at the start of play and every time a tile is placed
void newtile()
{
	tiletype = rand() % 3;
	tileorient = rand() % 4;

	// Update the geometry VBO of current tile
	
	if(tiletype==0){
		for (int i = 0; i < 4; i++)
		tile[i] = allRotationsIshape[tileorient][i]; // Get the 4 pieces of the new tile 
		updatetile();
	}
	else if(tiletype==1){
		for (int i = 0; i < 4; i++)
		tile[i] = allRotationsSshape[tileorient][i]; // Get the 4 pieces of the new tile
		updatetile();
	}
	else{
		for (int i = 0; i < 4; i++)
		tile[i] = allRotationsLshape[tileorient][i]; // Get the 4 pieces of the new tile
		updatetile();
	}


	// Update the color VBO of current tile
	tilecolor = int_to_color(rand() % 5);
	for (int i = 0; i < 144; i++) newcolours[i] = tilecolor;
	updatetile();

	glBindVertexArray(0);
}
//-------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------

// Places the current tile - update the board vertex colour VBO and the array maintaining occupied cells
void settile()
{
	int boardx;
	int boardy;

	for(int j = 0; j < 4; j++)
	{
		tilepos.x = round(getx());
		tilepos.y = round(gety());

		boardx = tilepos.x + tile[j].x; 
		boardy = tilepos.y + tile[j].y;
		board[boardx][boardy] = true;
		for (int i = (boardx*6)+(boardy*60); i < (boardx*6)+(boardy*60)+6; i++)
		{
			boardcolours[i] = newcolours[j*36];
		}
	}

	updateboard();
	newtile();
}


//-------------------------------------------------------------------------------------------------------------------

void initGrid()
{
		// ***Generate geometry data
	vec4 gridpoints[128]; // Array containing the 64 points of the 32 total lines to be later put in the VBO
	vec4 gridcolours[128]; // One colour per vertex
	// Vertical lines 
	for (int i = 0; i < 11; i++){
		gridpoints[2*i] = vec4(((1.0 * i)), 0.0, -0.5, 1);
		gridpoints[2*i + 1] = vec4(((1.0 * i)), 20.0, -0.5, 1);
		
	}
	// Horizontal lines
	for (int i = 0; i < 21; i++){
		gridpoints[22 + 2*i] = vec4(0, (1.0 * i), -0.5, 1);
		gridpoints[22 + 2*i + 1] = vec4(10.0, (1.0 * i), -0.5, 1);
	}

	// Vertical lines (second set)
	for (int i = 0; i < 11; i++){
		gridpoints[64 + 2*i] = vec4(((1.0 * i)), 0.0, .5, 1);
		gridpoints[64 + 2*i + 1] = vec4(((1.0 * i)), 20.0, .5, 1);
		
	}
	// Horizontal lines (second set)
	for (int i = 0; i < 21; i++){
		gridpoints[64 + 22 + 2*i] = vec4(0, (1.0 * i), 0.5, 1);
		gridpoints[64 + 22 + 2*i + 1] = vec4(10.0, (1.0 * i), 0.5, 1);
	}


	// Make all grid lines white
	for (int i = 0; i < 128; i++)
		gridcolours[i] = white;


	// *** set up buffer objects
	// Set up first VAO (representing grid lines)
	glBindVertexArray(vaoIDs[0]); // Bind the first VAO
	glGenBuffers(2, vboIDs); // Create two Vertex Buffer Objects for this VAO (positions, colours)

	// Grid vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[0]); // Bind the first grid VBO (vertex positions)
	glBufferData(GL_ARRAY_BUFFER, 128*sizeof(vec4), gridpoints, GL_STATIC_DRAW); // Put the grid points in the VBO
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0); 
	glEnableVertexAttribArray(vPosition); // Enable the attribute
	
	// Grid vertex colours
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[1]); // Bind the second grid VBO (vertex colours)
	glBufferData(GL_ARRAY_BUFFER, 128*sizeof(vec4), gridcolours, GL_STATIC_DRAW); // Put the grid colours in the VBO
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor); // Enable the attribute
}


void initBoard()
{
	// *** Generate the geometric data
	vec4 boardpoints[1200];
	for (int i = 0; i < 1200; i++)
		boardcolours[i] = black; // Let the empty cells on the board be black

	// Each cell is a square (2 triangles with 6 vertices)
	for (int i = 0; i < 20; i++){
		for (int j = 0; j < 10; j++)
		{		
			vec4 p1 = vec4((j * 1.0), (i * 1.0), .5, 1);
			vec4 p2 = vec4((j * 1.0), 1.0 + (i * 1.0), .5, 1);
			vec4 p3 = vec4(1.0 + (j * 1.0), (i * 1.0), .5, 1);
			vec4 p4 = vec4(1.0 + (j * 1.0), 1.0 + (i * 1.0), .5, 1);
			
			// Two points are reused
			boardpoints[6*(10*i + j)    ] = p1;
			boardpoints[6*(10*i + j) + 1] = p2;
			boardpoints[6*(10*i + j) + 2] = p3;
			boardpoints[6*(10*i + j) + 3] = p2;
			boardpoints[6*(10*i + j) + 4] = p3;
			boardpoints[6*(10*i + j) + 5] = p4;
		}
	}

	// Initially no cell is occupied
	for (int i = 0; i < 10; i++)
		for (int j = 0; j < 20; j++)
			board[i][j] = false; 


	// *** set up buffer objects
	glBindVertexArray(vaoIDs[1]);
	glGenBuffers(2, &vboIDs[2]);

	// Grid cell vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[2]);
	glBufferData(GL_ARRAY_BUFFER, 1200*sizeof(vec4), boardpoints, GL_STATIC_DRAW);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);

	// Grid cell vertex colours
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);
	glBufferData(GL_ARRAY_BUFFER, 1200*sizeof(vec4), boardcolours, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);
}

// No geometry for current tile initially
void initCurrentTile()
{
	glBindVertexArray(vaoIDs[2]);
	glGenBuffers(2, &vboIDs[4]);

	// Current tile vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[4]);
	glBufferData(GL_ARRAY_BUFFER, 144*sizeof(vec4), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);

	// Current tile vertex colours
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[5]);
	glBufferData(GL_ARRAY_BUFFER, 144*sizeof(vec4), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);
}

void initRobot()
{
	colorcube();

	//set up buffer objects
	glBindVertexArray(vaoIDs[3]);
	glGenBuffers(2, &vboIDs[6]);

	//robot arm vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[6]);
	glBufferData(GL_ARRAY_BUFFER, 36*sizeof(vec4), points, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);
	//robot arm colors
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[7]);
	glBufferData(GL_ARRAY_BUFFER, 36*sizeof(vec4), colors, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);
}

void init()
{
	// Load shaders and use the shader program
	GLuint program = InitShader("vshader.glsl", "fshader.glsl");
	glUseProgram(program);

	// Get the location of the attributes (for glVertexAttribPointer() calls)
	vPosition = glGetAttribLocation(program, "vPosition");
	vColor = glGetAttribLocation(program, "vColor");

	// Create 3 Vertex Array Objects, each representing one 'object'. Store the names in array vaoIDs
	glGenVertexArrays(4, &vaoIDs[0]);

	// Initialize the grid, the board, and the current tile
	initGrid();
	initBoard();
	initCurrentTile();
	initRobot();

	// The location of the uniform variables in the shader program
	locxsize = glGetUniformLocation(program, "xsize"); 
	locysize = glGetUniformLocation(program, "ysize");

	model_view = glGetUniformLocation( program, "model_view" );
    projection = glGetUniformLocation( program, "projection" );
    
    glEnable( GL_DEPTH_TEST );

	// Game initialization
	newtile(); // create new next tile

	// set to default
	glBindVertexArray(0);
	glClearColor(0, 0, 0, 0);
}

//-------------------------------------------------------------------------------------------------------------------
// Rotates the current tile, if there is room
void rotate()
{      
	if(tiletype == 0)
	{
		for (int i = 0; i < 4; i++) 
		{
			tile[i] = allRotationsIshape[(tileorient+1) % 4][i];
		}
		tileorient = (tileorient+1) % 4;
	}
	else if(tiletype == 1)
	{
		for (int i = 0; i < 4; i++) 
		{
			tile[i] = allRotationsSshape[(tileorient+1) % 4][i];
		}
		tileorient = (tileorient+1) % 4;
	}
	else if(tiletype == 2)
	{
		for (int i = 0; i < 4; i++) 
		{
			tile[i] = allRotationsLshape[(tileorient+1) % 4][i];
		}
		tileorient = (tileorient+1) % 4;
	}

	updatetile();
}

//-------------------------------------------------------------------------------------------------------------------

// Starts the game over - empties the board, creates new tiles, resets line counters
void restart()
{
	for (int i = 0; i < 1200; i++)
		boardcolours[i] = black; // Let the empty cells on the board be black

	for (int i = 0; i < 10; i++)
		for (int j = 0; j < 20; j++)
			board[i][j] = false;

	newtile();
	updateboard();

}
//-------------------------------------------------------------------------------------------------------------------

// Draws the game
void display()
{

//------------------
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    vec4  eye(5+radius*sin(cameraangle), 20.0, radius*cos(cameraangle), 1.0 );
    //vec4  eye(200, 200, 200, 1.0 );
    
    vec4 at( 5.0, 10.0, 0.0, 1.0 );
    //vec4 at( 300, 300, 0.0, 1.0 );
    vec4 up( 0.0, 1.0, 0.0, 0.0 );

    mv = LookAt( eye, at, up );
    glUniformMatrix4fv( model_view, 1, GL_TRUE, mv );

    mat4  p = Perspective(45, xsize/ysize ,zNear, zFar );
    glUniformMatrix4fv( projection, 1, GL_TRUE, p );

	glUniform1i(locxsize, xsize); // x and y sizes are passed to the shader program to maintain shape of the vertices on screen
	glUniform1i(locysize, ysize);

	glBindVertexArray(vaoIDs[1]); // Bind the VAO representing the grid cells (to be drawn first)
	glDrawArrays(GL_TRIANGLES, 0, 1200); // Draw the board (10*20*2 = 400 triangles)

	glBindVertexArray(vaoIDs[2]); // Bind the VAO representing the current tile (to be drawn on top of the board)
	glDrawArrays(GL_TRIANGLES, 0, 144); // Draw the current tile (8 triangles)

	glBindVertexArray(vaoIDs[0]); // Bind the VAO representing the grid lines (to be drawn on top of everything else)
	glDrawArrays(GL_LINES, 0, 128); // Draw the grid lines (21+11 = 32 lines)

	glBindVertexArray(vaoIDs[3]);

	//base
    mv *= RotateY(Theta[Base] );
    mat4 instance = ( Translate( BASE_OFFSET, 0.5 * BASE_HEIGHT, 0.0 ) *
		 Scale( BASE_WIDTH,
			BASE_HEIGHT,
			BASE_WIDTH ) );

    glUniformMatrix4fv( model_view, 1, GL_TRUE, mv * instance );
    glDrawArrays( GL_TRIANGLES, 0, NumVertices );


    //lower arm
    instance = ( Translate( 0.0, 0.5 * LOWER_ARM_HEIGHT, 0.0 ) *
		      Scale( LOWER_ARM_WIDTH,
			     LOWER_ARM_HEIGHT,
			     LOWER_ARM_WIDTH ) );
    mv *= ( Translate(BASE_OFFSET, BASE_HEIGHT, 0.0) * RotateZ(Theta[LowerArm]) );
    
    glUniformMatrix4fv( model_view, 1, GL_TRUE, mv * instance );
    glDrawArrays( GL_TRIANGLES, 0, NumVertices );

    //upper arm
    
    instance = ( Translate( 0.0, 0.5 * UPPER_ARM_HEIGHT, 0.0 ) *
		      Scale( UPPER_ARM_WIDTH,
			     UPPER_ARM_HEIGHT,
			     UPPER_ARM_WIDTH ) );
    mv *= ( Translate(0.0, LOWER_ARM_HEIGHT, 0.0) * RotateZ(Theta[UpperArm]) );
    
    glUniformMatrix4fv( model_view, 1, GL_TRUE, mv * instance );
    glDrawArrays( GL_TRIANGLES, 0, NumVertices );


	glutSwapBuffers();
}

//-------------------------------------------------------------------------------------------------------------------

// Reshape callback will simply change xsize and ysize variables, which are passed to the vertex shader
// to keep the game the same from stretching if the window is stretched
void reshape(GLsizei w, GLsizei h)
{
	xsize = w;
	ysize = h;
	glViewport(0, 0, w, h);
}

//-------------------------------------------------------------------------------------------------------------------

// Handle arrow key keypresses
void special(int key, int x, int y)
{
	if(glutGetModifiers() == GLUT_ACTIVE_CTRL)
	{
		switch(key)
		{
			case GLUT_KEY_LEFT: 
			cameraangle += dr; 
			break;

			case GLUT_KEY_RIGHT: 
			cameraangle -= dr; 
			break;
		}
	}
	else if(glutGetModifiers() != GLUT_ACTIVE_CTRL)
	{
		switch(key) 
		{
			case GLUT_KEY_UP:
			    rotate();
			    break;
		}
	}

	glutPostRedisplay();
}

//-------------------------------------------------------------------------------------------------------------------

// Handles standard keypresses
void keyboard(unsigned char key, int x, int y)
{
	switch(key) 
	{
		case 033: // Both escape key and 'q' cause the game to exit
		    exit(EXIT_SUCCESS);
		    break;
		case 'q':
			exit (EXIT_SUCCESS);
			break;
		case 'r': // 'r' key restarts the game
			restart();
			break;

		case 'a':
			Theta[LowerArm] += 3.0;
			if ( Theta[LowerArm] > 360.0 ) { Theta[LowerArm] -= 360.0; }
			getx();
			updatetile();
			break;

		case 'd':
			Theta[LowerArm] -= 3.0;
			if ( Theta[LowerArm] < 0.0 ) { Theta[LowerArm] += 360.0; }
			getx();
			updatetile();
			break;

		case 'w':
			Theta[UpperArm] += 3.0;
			if ( Theta[UpperArm] > 360.0 ) { Theta[UpperArm] -= 360.0; }
			getx();
			updatetile();
			break;

		case 's':
			Theta[UpperArm] -= 3.0;
			if ( Theta[UpperArm] < 0.0 ) { Theta[UpperArm] += 360.0; }
			getx();
			updatetile();
			break;

		case ' ':
			if(canplace()) 
			{
				settile();
			}
			break;
	}
	glutPostRedisplay();
}

//----------------------------------------------------------------------------

void
mouse( int button, int state, int x, int y )
{

    if ( button == GLUT_LEFT_BUTTON && state == GLUT_DOWN ) {
	// Incrase the joint angle
	Theta[Axis] += 5.0;
	if ( Theta[Axis] > 360.0 ) { Theta[Axis] -= 360.0; }
    }

    if ( button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN ) {
	// Decrase the joint angle
	Theta[Axis] -= 5.0;
	if ( Theta[Axis] < 0.0 ) { Theta[Axis] += 360.0; }
    }

    glutPostRedisplay();
}

//-------------------------------------------------------------------------------------------------------------------

void idle(void)
{
	glutPostRedisplay();
}

//-------------------------------------------------------------------------------------------------------------------
int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(xsize, ysize);
	glutInitWindowPosition(680, 178); // Center the game window (well, on a 1920x1080 display)
	glutCreateWindow("Fruit Tetris");
	glewInit();
	init();

	// Callback functions
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutSpecialFunc(special);
	glutKeyboardFunc(keyboard);
	glutMouseFunc( mouse );


	glutIdleFunc(idle);

	glutMainLoop(); // Start main loop
	return 0;
}
