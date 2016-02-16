void tilegravity(int fallspeed)
{
	glutTimerFunc(1000, tilegravity, 1);
	tilepos.y = tilepos.y - fallspeed;

	//int boardx;
	int boardy;

	for (int i = 0; i < 4; i++) 
	{
		//boardx = tilepos.x + tile[i].x;
		boardy = tilepos.y + tile[i].y;

		if(boardy == 0)
		{
			settile();
			break;
		}
	}
	cout << "next" << endl;

	updatetile();
	glutPostRedisplay();
}

//-------------------------------------------------------------------------------------------------------------------
// 
//gravity and ground collision detection

void tilegravity(int fallspeed)
{
	glutTimerFunc(1000, tilegravity, 1);
	movetiledown();

	//int boardx;
	int boardy;

	for (int i = 0; i < 4; i++) 
	{
		//boardx = tilepos.x + tile[i].x;
		boardy = tilepos.y + tile[i].y;

		if(boardy == 0)
		{
			settile();
			break;
		}
	}
	cout << "next" << endl;

	updatetile();
	glutPostRedisplay();
}

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
		return true;
	}
	return false;
}

//-------------------------------------------------------------------------------------------------------------------
// Called at the start of play and every time a tile is placed
void newtile()
{
	//tiletype = rand() % 3;
	tiletype = 0;
	tileorient = rand() % 4;
	tilepos = vec2(5 , 18); // Put the tile at the top of the board

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
	if(choosetile==0){
		for (int i = 0; i < 4; i++)
		tile[i] = allRotationsIshape[randorient][i]; // Get the 4 pieces of the new tile
		updatetile(); 
	}
	else if(choosetile==1){
		for (int i = 0; i < 4; i++)
		tile[i] = allRotationsSshape[randorient][i]; // Get the 4 pieces of the new tile
		updatetile();
	}
	else{
		for (int i = 0; i < 4; i++)
		tile[i] = allRotationsLshape[randorient][i]; // Get the 4 pieces of the new tile
		updatetile();
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
		return true;
	}
	return false;
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