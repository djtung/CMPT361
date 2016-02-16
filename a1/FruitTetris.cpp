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


// xsize and ysize represent the window size - updated if window is reshaped to prevent stretching of the game
int xsize = 400; 
int ysize = 720;

// current tile
vec2 tile[4]; // An array of 4 2d vectors representing displacement from a 'center' piece of the tile, on the grid
vec2 tilepos = vec2(5, 19); // The position of the current tile using grid coordinates ((0,0) is the bottom left corner)

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



// colors
vec4 white  = vec4(1.0, 1.0, 1.0, 1.0);
vec4 black  = vec4(0.0, 0.0, 0.0, 1.0); // 5 or any (default color)

vec4 purple = vec4(1.0, 0.0, 1.0, 1.0); // 0
vec4 red = vec4(1.0, 0.0, 0.0, 1.0); // 1
vec4 yellow = vec4(1.0, 1.0, 0.0, 1.0); // 2
vec4 green = vec4(0.0, 1.0, 0.0, 1.0); // 3
vec4 orange = vec4(1.0, 0.5, 0.0, 1.0); // 4

//current tile colours
vec4 newcolours[24];

 
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
GLuint vaoIDs[3]; // One VAO for each object: the grid, the board, the current piece
GLuint vboIDs[6]; // Two Vertex Buffer Objects for each VAO (specifying vertex positions and colours, respectively)

//-------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------
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
// compares colours to see if they match
// must be a color, black returns false
bool comparecolours(vec4 a, vec4 b, vec4 c)
{
	if((a.x == black.x) && (a.y == black.y) && (a.z == black.z) && (a.w == black.w)){
		return false;
	}

	if((a.x == b.x) && (a.y == b.y) && (a.z == b.z) && (a.w == b.w)){
		if((a.x == c.x) && (a.y == c.y) && (a.z == c.z) && (a.w == c.w))
		{
			return true;
		}
		else{
			return false;
		}
	} 
	else{
		return false;
	}
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
		GLfloat x = tilepos.x + tile[i].x; 
		GLfloat y = tilepos.y + tile[i].y;

		// Create the 4 corners of the square - these vertices are using location in pixels
		// These vertices are later converted by the vertex shader
		vec4 p1 = vec4(33.0 + (x * 33.0), 33.0 + (y * 33.0), .4, 1); 
		vec4 p2 = vec4(33.0 + (x * 33.0), 66.0 + (y * 33.0), .4, 1);
		vec4 p3 = vec4(66.0 + (x * 33.0), 33.0 + (y * 33.0), .4, 1);
		vec4 p4 = vec4(66.0 + (x * 33.0), 66.0 + (y * 33.0), .4, 1);

		// Two points are used by two triangles each
		vec4 newpoints[6] = {p1, p2, p3, p2, p3, p4}; 

		// Put new data in the VBO
		glBufferSubData(GL_ARRAY_BUFFER, i*6*sizeof(vec4), 6*sizeof(vec4), newpoints); 
	}

	glBindVertexArray(0);
}

//-------------------------------------------------------------------------------------------------------------------
//shifts down 1 row from column x to column y, starting at row z (z replaced by above)
// columns are 0 to 9, rows 0 to 19
void shiftdown(int x, int y, int z){
	bool tempboard[10][20];
	vec4 tempboardcolours[1200];
	int i, j;

	//fill temps
	for (i = 0; i < 10; i++)
		for (j = 0; j < 20; j++)
			tempboard[i][j] = board[i][j];

	for(i = 0; i <1200; i++) tempboardcolours[i] = boardcolours[i];

	// new values
	for (i = x; i <= y; i++)
		for (j = z; j < 19; j++)
			tempboard[i][j] = board[i][j+1];
	for(i = 0; i < 10; i++) tempboard[i][19] = false;


	for(j = z; j < 19; j++)
	{
		for(i = 6*(10*j+x); i < 6*(10*j+(1+y)); i++) tempboardcolours[i] = boardcolours[i+60];
	}
	for(i = 1140+(x*6); i < 1140+((1+y)*6); i++) tempboardcolours[i] = black;
	
	//refill board
	for (i = 0; i < 10; i++)
		for (j = 0; j < 20; j++)
			board[i][j] = tempboard[i][j];	

	for(i = 0; i < 1200; i++) boardcolours[i] = tempboardcolours[i];
}

//-------------------------------------------------------------------------------------------------------------------
//shifts down a column x at row z an amount of 'amount'
//columns 0 to 9, rows 0 to 19
void shiftdowncolumn(int x, int z, int amount){
	bool tempboard[10][20];
	vec4 tempboardcolours[1200];
	int i, j;

	//fill temps
	for (i = 0; i < 10; i++)
		for (j = 0; j < 20; j++)
			tempboard[i][j] = board[i][j];

	for(i = 0; i <1200; i++) tempboardcolours[i] = boardcolours[i];

	// new values
	for (j = z; j < 19-amount; j++)
		tempboard[x][j] = board[x][j+amount];
	for(i = 19-amount; i < 20; i++) tempboard[x][i] = false;

	for(j = z; j < 19-amount; j++)
	{
		for(i = 0; i < 6 ; i++) tempboardcolours[i+(x*6)+(j*60)] = boardcolours[i+(x*6)+(j*60)+60*amount];
	}
	for(i = 60*(19-amount)+x*6; i < 1140+x*6; i=i+60) tempboardcolours[i] = black;
	
	//refill board
	for (i = 0; i < 10; i++)
		for (j = 0; j < 20; j++)
			board[i][j] = tempboard[i][j];	

	for(i = 0; i < 1200; i++) boardcolours[i] = tempboardcolours[i];
}
//-------------------------------------------------------------------------------------------------------------------

// Checks if the specified row (0 is the bottom 19 the top) is full
// If every cell in the row is occupied, it will clear that cell and everything above it will shift down one row
void checkfullrow(int row)
{
	bool rowhasempty = false;
	for(int i = 0; i < 10; i++)
	{
		if(board[i][row] == false)
		{
			rowhasempty = true;
		}
	}
	if(!rowhasempty)
	{
		shiftdown(0, 9, row);
	}
}


//-------------------------------------------------------------------------------------------------------------------
//checks all fruits of groups >3 and removes them
void checkfruits(){

	bool toremove[10][20];
	bool clusterexist = true;
	int i, j;
	for(i = 0; i < 10; i++)
	{
		for (j=0; j < 20; j++) toremove[i][j] = false;
	}
	while(clusterexist)
	{
		clusterexist = false;
		//check rows
		for(j=0; j<20; j++)
		{
			for(i=0; i<9; i++)
			{
				if(comparecolours(boardcolours[j*60+i*6], boardcolours[j*60+i*6+6], boardcolours[j*60+i*6+12]))
				{
					toremove[i][j] = true;
					toremove[i+1][j] = true;
					toremove[i+2][j] = true;
					clusterexist = true;
				}
			}
		}
		//check columns
		for(j=0; j<10; j++)
		{
			for(i=0; i<18; i++)
			{
				if(comparecolours(boardcolours[j*6+i*60], boardcolours[j*6+(i+1)*60], boardcolours[j*6+(i+2)*60]))
				{
					toremove[j][i] = true;
					toremove[j][i+1] = true;
					toremove[j][i+2] = true;
					clusterexist = true;
				}
			}
		}

		//remove
		if(clusterexist)
		{
			for(i = 0; i < 10; i++)
			{
				for (j=19; j >= 0; j--)
				{
					if(toremove[i][j])
					{
						shiftdowncolumn(i, j, 1);
						toremove[i][j] = false;
					}
				}
			}
		}
	}
}

//-------------------------------------------------------------------------------------------------------------------


//updates the board
void updateboard() {
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);
	glBufferData(GL_ARRAY_BUFFER, 1200*sizeof(vec4), boardcolours, GL_DYNAMIC_DRAW);
}


//-------------------------------------------------------------------------------------------------------------------
//check if the current tile shape and orientation can fit on the game board
bool canfit(){
	int newboardx; 
	int newboardy;

	if(tiletype == 0)
	{
		for (int i = 0; i < 4; i++) 
		{
			newboardx = tilepos.x + allRotationsIshape[tileorient][i].x;
			newboardy = tilepos.y + allRotationsIshape[tileorient][i].y;
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
			newboardx = tilepos.x + allRotationsSshape[tileorient][i].x;
			newboardy = tilepos.y + allRotationsSshape[tileorient][i].y;

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
			newboardx = tilepos.x + allRotationsLshape[tileorient][i].x;
			newboardy = tilepos.y + allRotationsLshape[tileorient][i].y;

			if(board[newboardx][newboardy] == 1 || newboardx < 0 || newboardx > 9 || newboardy < 0 || newboardy > 19)
			{
				return false;
			}
		}
	}
	return true;
}

//-------------------------------------------------------------------------------------------------------------------
// Called at the start of play and every time a tile is placed
void newtile()
{
	int counter = 0;
	bool tilecanfit = true;
	tiletype = rand() % 3;
	tileorient = rand() % 4;
	int startingpos = 2 + (rand() % 7);

	if(tileorient == 1 || tileorient == 3 || (tiletype==2 && tileorient != 0))
	{
		tilepos = vec2(startingpos, 18);
	}
	else
	{
		tilepos = vec2(startingpos, 19); // Put the tile at the top of the board
	}

	tilecanfit = canfit();
	while(!tilecanfit)
	{
		tiletype = rand() % 3;
		tileorient = rand() % 4;
		int startingpos = 2 + (rand() % 7);

		if(tileorient == 1 || tileorient == 3 || (tiletype==2 && tileorient != 0))
		{
			tilepos = vec2(startingpos, 18);
		}
		else
		{
			tilepos = vec2(startingpos, 19); // Put the tile at the top of the board
		}

		tilecanfit = canfit();
		counter++;

		if(counter == 100)
		{
			cout << endl;
			cout << "/---- GAME OVER ---- /" << endl;
			cout << endl;
			exit (EXIT_SUCCESS);
		}
	}

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
	vec4 tilecolor = int_to_color(rand() % 5);
	for (int i = 0; i < 6; i++) newcolours[i] = tilecolor;
	tilecolor = int_to_color(rand() % 5);
	for (int i = 6; i < 12; i++) newcolours[i] = tilecolor;
	tilecolor = int_to_color(rand() % 5);
	for (int i = 12; i < 18; i++) newcolours[i] = tilecolor;
	tilecolor = int_to_color(rand() % 5);
	for (int i = 18; i < 24; i++) newcolours[i] = tilecolor;

	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[5]); // Bind the VBO containing current tile vertex colours
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(newcolours), newcolours); // Put the colour data in the VBO
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}
//-------------------------------------------------------------------------------------------------------------------
// shuffles tile color within the tile
void shufflefruit()
{
	vec4 temp[24];
	for( int i = 0; i < 24; i++)
	{
		temp[(i+6) % 24] = newcolours[i];
	}
	for( int i = 0; i < 24; i++)
	{
		newcolours[i] = temp[i];
	}
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[5]); // Bind the VBO containing current tile vertex colours
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(newcolours), newcolours); // Put the colour data in the VBO
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}

//-------------------------------------------------------------------------------------------------------------------

// Places the current tile - update the board vertex colour VBO and the array maintaining occupied cells
void settile()
{
	int boardx;
	int boardy;
	int miny = 30;
	int maxy = 0;

	for(int j = 0; j < 4; j++)
	{
		boardx = tilepos.x + tile[j].x; 
		boardy = tilepos.y + tile[j].y;
		board[boardx][boardy] = true;
		for (int i = (boardx*6)+(boardy*60); i < (boardx*6)+(boardy*60)+6; i++)
		{
			boardcolours[i] = newcolours[j*6];
		}
		if(boardy<miny) miny = boardy;
		if(boardy>maxy) maxy = boardy;
	}

	for(int k = maxy; k >= miny; k--){
		checkfullrow(k);
		updateboard();
	}
	checkfruits();
	updateboard();
	newtile();
}

//-------------------------------------------------------------------------------------------------------------------
//checks to see if the tile can move down 1 block, and does so if it can
//returns true for a successful move and false for no movement
bool movetiledown()
{
	bool canmove = true;

	for (int i = 0; i < 4; i++) 
	{
		int boardx = tilepos.x + tile[i].x;
		int boardy = tilepos.y + tile[i].y;

		if(board[boardx][boardy-1] == 1 || boardy == 0)
		{
			canmove = false;
		}
	}
	
	if(canmove){
		tilepos.y = tilepos.y - 1;
		updatetile();
		return true;
	}
	return false;
}

//-------------------------------------------------------------------------------------------------------------------

// Given x, tries to move the tile x squares to the right
// Returns true if the tile was successfully moved, or false if there was some issue
bool movetilelateral(int x)
{
	bool canmove = true;

	for (int i = 0; i < 4; i++) 
	{
		int boardx = tilepos.x + tile[i].x;
		int boardy = tilepos.y + tile[i].y;

		if(board[boardx+x][boardy] == 1 || boardx+x < 0 || boardx+x > 9)
		{
			canmove = false;
		}
	}
	
	if(canmove){
		tilepos.x = tilepos.x + x;
		updatetile();
		return true;
	}
	return false;
}

//-------------------------------------------------------------------------------------------------------------------
// 
//gravity and ground collision detection
//game init

void tilegravity(int fallspeed)
{
	glutTimerFunc(700, tilegravity, 1);
	bool candrop = movetiledown();

	if(!candrop){
		settile();
	}

	updatetile();
	glutPostRedisplay();
}

//-------------------------------------------------------------------------------------------------------------------

void initGrid()
{
	// ***Generate geometry data
	vec4 gridpoints[64]; // Array containing the 64 points of the 32 total lines to be later put in the VBO
	vec4 gridcolours[64]; // One colour per vertex
	// Vertical lines 
	for (int i = 0; i < 11; i++){
		gridpoints[2*i] = vec4((33.0 + (33.0 * i)), 33.0, 0, 1);
		gridpoints[2*i + 1] = vec4((33.0 + (33.0 * i)), 693.0, 0, 1);
		
	}
	// Horizontal lines
	for (int i = 0; i < 21; i++){
		gridpoints[22 + 2*i] = vec4(33.0, (33.0 + (33.0 * i)), 0, 1);
		gridpoints[22 + 2*i + 1] = vec4(363.0, (33.0 + (33.0 * i)), 0, 1);
	}
	// Make all grid lines white
	for (int i = 0; i < 64; i++)
		gridcolours[i] = white;


	// *** set up buffer objects
	// Set up first VAO (representing grid lines)
	glBindVertexArray(vaoIDs[0]); // Bind the first VAO
	glGenBuffers(2, vboIDs); // Create two Vertex Buffer Objects for this VAO (positions, colours)

	// Grid vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[0]); // Bind the first grid VBO (vertex positions)
	glBufferData(GL_ARRAY_BUFFER, 64*sizeof(vec4), gridpoints, GL_STATIC_DRAW); // Put the grid points in the VBO
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0); 
	glEnableVertexAttribArray(vPosition); // Enable the attribute
	
	// Grid vertex colours
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[1]); // Bind the second grid VBO (vertex colours)
	glBufferData(GL_ARRAY_BUFFER, 64*sizeof(vec4), gridcolours, GL_STATIC_DRAW); // Put the grid colours in the VBO
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
			vec4 p1 = vec4(33.0 + (j * 33.0), 33.0 + (i * 33.0), .5, 1);
			vec4 p2 = vec4(33.0 + (j * 33.0), 66.0 + (i * 33.0), .5, 1);
			vec4 p3 = vec4(66.0 + (j * 33.0), 33.0 + (i * 33.0), .5, 1);
			vec4 p4 = vec4(66.0 + (j * 33.0), 66.0 + (i * 33.0), .5, 1);
			
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
	glBufferData(GL_ARRAY_BUFFER, 24*sizeof(vec4), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);

	// Current tile vertex colours
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[5]);
	glBufferData(GL_ARRAY_BUFFER, 24*sizeof(vec4), NULL, GL_DYNAMIC_DRAW);
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
	glGenVertexArrays(3, &vaoIDs[0]);

	// Initialize the grid, the board, and the current tile
	initGrid();
	initBoard();
	initCurrentTile();

	// The location of the uniform variables in the shader program
	locxsize = glGetUniformLocation(program, "xsize"); 
	locysize = glGetUniformLocation(program, "ysize");

	// Game initialization
	newtile(); // create new next tile
	tilegravity(1);

	// set to default
	glBindVertexArray(0);
	glClearColor(0, 0, 0, 0);
}
//-------------------------------------------------------------------------------------------------------------------
//check if the current tile shape can rotate (next rotation has no colissions)
bool canrotate(){
	int newboardx; 
	int newboardy;

	if(tiletype == 0)
	{
		for(int j = 0; j < 4; j++)
		{
			if(tileorient == j)
			{
				for (int i = 0; i < 4; i++) 
				{
					newboardx = tilepos.x + allRotationsIshape[(j+1)%4][i].x;
					newboardy = tilepos.y + allRotationsIshape[(j+1)%4][i].y;

					if(board[newboardx][newboardy] == 1 || newboardx < 0 || newboardx > 9 || newboardy < 0 || newboardy > 19)
					{
						return false;
					}
				}
			}
		}
	}
	if(tiletype == 1)
	{
		for(int j = 0; j < 4; j++)
		{
			if(tileorient == j)
			{
				for (int i = 0; i < 4; i++) 
				{
					newboardx = tilepos.x + allRotationsSshape[(j+1)%4][i].x;
					newboardy = tilepos.y + allRotationsSshape[(j+1)%4][i].y;

					if(board[newboardx][newboardy] == 1 || newboardx < 0 || newboardx > 9 || newboardy < 0 || newboardy > 19)
					{
						return false;
					}
				}
			}
		}
	}
	if(tiletype == 2)
	{
		for(int j = 0; j < 4; j++)
		{
			if(tileorient == j)
			{
				for (int i = 0; i < 4; i++) 
				{
					newboardx = tilepos.x + allRotationsLshape[(j+1)%4][i].x;
					newboardy = tilepos.y + allRotationsLshape[(j+1)%4][i].y;

					if(board[newboardx][newboardy] == 1 || newboardx < 0 || newboardx > 9 || newboardy < 0 || newboardy > 19)
					{
						return false;
					}
				}
			}
		}
	}
	return true;
}


//-------------------------------------------------------------------------------------------------------------------
// Rotates the current tile, if there is room
void rotate()
{      
	if(canrotate() && tiletype == 0)
	{
		for (int i = 0; i < 4; i++) 
		{
			tile[i] = allRotationsIshape[(tileorient+1) % 4][i];
		}
		tileorient = (tileorient+1) % 4;
	}
	else if(canrotate() && tiletype == 1)
	{
		for (int i = 0; i < 4; i++) 
		{
			tile[i] = allRotationsSshape[(tileorient+1) % 4][i];
		}
		tileorient = (tileorient+1) % 4;
	}
	else if(canrotate() && tiletype == 2)
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

	glClear(GL_COLOR_BUFFER_BIT);

	glUniform1i(locxsize, xsize); // x and y sizes are passed to the shader program to maintain shape of the vertices on screen
	glUniform1i(locysize, ysize);

	glBindVertexArray(vaoIDs[1]); // Bind the VAO representing the grid cells (to be drawn first)
	glDrawArrays(GL_TRIANGLES, 0, 1200); // Draw the board (10*20*2 = 400 triangles)

	glBindVertexArray(vaoIDs[2]); // Bind the VAO representing the current tile (to be drawn on top of the board)
	glDrawArrays(GL_TRIANGLES, 0, 24); // Draw the current tile (8 triangles)

	glBindVertexArray(vaoIDs[0]); // Bind the VAO representing the grid lines (to be drawn on top of everything else)
	glDrawArrays(GL_LINES, 0, 64); // Draw the grid lines (21+11 = 32 lines)


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
	switch(key) 
	{
		case GLUT_KEY_LEFT:
		    movetilelateral(-1);
		    break;
		case GLUT_KEY_RIGHT:
		    movetilelateral(1);
		    break;
		case GLUT_KEY_DOWN:
		    movetiledown();
		    break;
		case GLUT_KEY_UP:
		    rotate();
		    break;
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
		case ' ':
			shufflefruit();
			break;

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
	glutIdleFunc(idle);

	glutMainLoop(); // Start main loop
	return 0;
}
