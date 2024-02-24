/*
 * game.c
 *
 * Functionality related to the game state and features.
 *
 * Author: Jarrod Bennett, Cody Burnett
 * Modified: Kavisha Welg
 */ 

#include "game.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "display.h"
#include "terminalio.h"
#include "timer0.h"
#include "ledmatrix.h"


// Player paddle positions. y coordinate refers to lower pixel on paddle.
// x coordinates never change but are nice to have here to use when drawing to
// the display.
static const int8_t PLAYER_X_COORDINATES[] = {PLAYER_1_X, PLAYER_2_X};
static int8_t player_y_coordinates[] = {0, 0};

// Ball position
int8_t ball_x;
int8_t ball_y;

// Ball direction
int8_t ball_x_direction;
int8_t ball_y_direction;

//Initialize the player scores:
int player1_score = 0;
int player2_score = 0 ;


//Initialize player1 and player2 rally numbers.
int player1_rally_number = 0;
int player2_rally_number = 0;


void draw_player_paddle(uint8_t player_to_draw);
void erase_player_paddle(uint8_t player_to_draw);

// Initialise the player paddles, ball and display to start a game of PONG.
void initialise_game(void) {
	
	// initialise the display we are using.
	initialise_display();

	// Start players in the middle of the board
	player_y_coordinates[PLAYER_1] = BOARD_HEIGHT / 2 - 1;
	player_y_coordinates[PLAYER_2] = BOARD_HEIGHT / 2 - 1;

	draw_player_paddle(PLAYER_1);
	draw_player_paddle(PLAYER_2);
	
	//Have the ball travel in a random direction when the game is started.
	
	//Generate a random number:
	srand(get_current_time()); //Seed the random number generator.
	
	//Generate arrays with the possible direction factors for x and y:
	int y_direction_array[] = {-1, 0 , 1};
	int x_direction_array[] = {-1, 1};
	
	int random_number_y  = rand() % 3 ; //Selects a random number which is either 0,1,2
	int random_number_x = rand() % 2;  //Selects two random numbers either 0,1.
	
	int direction_number_x = x_direction_array[random_number_x] ; //Randomly select an array number from the x_direction_array.
	int direction_number_y = y_direction_array [random_number_y] ; //Randomly select an array number from the y_direction_array.
	
	//Ball_x_direction and ball_y_direction are now equal to random numbers direction_number_x and direction_number_y.
	ball_x_direction = direction_number_x;
	ball_y_direction = direction_number_y;
		

	// Clear the old ball
	update_square_colour(ball_x, ball_y, EMPTY_SQUARE);
	
	// Reset ball position and direction
	ball_x = BALL_START_X;
	ball_y = BALL_START_Y;

	
	// Draw new ball
	update_square_colour(ball_x, ball_y, BALL);
}

// Draw player 1 or 2 on the game board at their current position (specified
// by the `PLAYER_X_COORDINATES` and `player_y_coordinates` variables).
// This makes it easier to draw the multiple pixels of the players.
void draw_player_paddle(uint8_t player_to_draw) {
	int8_t player_x = PLAYER_X_COORDINATES[player_to_draw];
	int8_t player_y = player_y_coordinates[player_to_draw];

	for (int y = player_y; y < player_y + PLAYER_HEIGHT; y++) {
		update_square_colour(player_x, y, PLAYER);
	}
}

// Erase the pixels of player 1 or 2 from the display.
void erase_player_paddle(uint8_t player_to_draw) {
	int8_t player_x = PLAYER_X_COORDINATES[player_to_draw];
	int8_t player_y = player_y_coordinates[player_to_draw];

	for (int y = player_y; y < player_y + PLAYER_HEIGHT; y++) {
		update_square_colour(player_x, y, EMPTY_SQUARE);
	}
}

// Try and move the selected player's y coordinate by the amount specified.
// For example, to move player 1's paddle up one space, call the function
// as `move_player(PLAYER_1, 1)`. Use `-1` instead to move the paddle down. No
// pixels of the player paddles should be allowed to move off the display.
void move_player_paddle(int8_t player, int8_t direction) {
	/* suggestions for implementation:
	 * 1: Figure out the new location of the player paddle. Consider what should
	 *	  happen if the player paddle is at the edge of the board.
	 * 2: Remove the player from the display at the current position.
	 * 3: Update the positional knowledge of the player. This will involve the
	 *    player coordinate variables.
	 * 4: Display the player at their new position.
	 */	
	
	// If the paddles are at the top and bottom of the board, then the paddles will not be able to move. 
	if ((player_y_coordinates[player] + direction > BOARD_HEIGHT -2 || player_y_coordinates[player] + direction < 0)){
			return; 
	}
	//Nested if statements for if the ball is right under or right above the paddle (on the same column that the paddle is on. This to stop the paddle moving onto the ball. 
	if (ball_x == PLAYER_1_X || ball_x == PLAYER_2_X){
		if (ball_y == player_y_coordinates[player] + 2 || ball_y == player_y_coordinates[player] -1 ){
			return; 
			}
	}
		
		erase_player_paddle(player);
		player_y_coordinates[player] += direction;
		draw_player_paddle(player);
	
	
}

void player_1_raller_counter(void){
	int player1_display_index;
	int board1_rally_display;
	
	//Set condition for when player1 has hit a rally of 8;
	if (player1_rally_number == 8) {
		player1_display_index = player1_rally_number -1; //The display index is the circle index on the column that will be coloured for the rally. 
	}
	
	//If the player1_rally_number exceeds the number that can be coloured in the rally counter. 
	else if (player1_rally_number > 8 ){
		//Clear the zeroth column.
		player1_rally_number = 1; 
		player1_display_index= 0; //Set the display index to 0 to later colour the first circle of the rally counter. 
		for (board1_rally_display = 0; board1_rally_display <= 7; board1_rally_display ++){ //Clear the column and make it black. 
			ledmatrix_update_pixel(0,board1_rally_display, COLOUR_BLACK);
			}
	}
	
	else {
		player1_display_index = (player1_rally_number % 8) -1;
	}
	
	//Colour the column up the player display index. 
	for (board1_rally_display = 0; board1_rally_display <= player1_display_index; board1_rally_display ++){
		ledmatrix_update_pixel(0,board1_rally_display, COLOUR_RALLY_COUNTER);
	}
	
}



void clear_rally1_counter(void){
	player1_rally_number = 0; 
	for (int board1_rally_display = 0; board1_rally_display <= 8; board1_rally_display ++){
		ledmatrix_update_pixel(0,board1_rally_display, COLOUR_BLACK);
	}
}


//Setting up a rally counter display for player 2. 
void player_2_raller_counter(void){
	int player2_display_index;
	int board2_rally_display;
	
	//Set condition for when player2 has hit a rally of 8;
	if (player2_rally_number == 8) {
		player2_display_index = player2_rally_number -1; //The display index is the circle index on the column that will be coloured for the rally.
	}
	
	//If the player1_rally_number exceeds the number that can be coloured in the rally counter.
	else if (player2_rally_number > 8){
		//Clear the zeroth column.
		player2_rally_number = 1;  //Reset the player rally number to 1;
		player2_display_index= 0; //Set the display index to 0 to later colour the first circle of the rally counter.
		for (board2_rally_display = 0; board2_rally_display <= 7; board2_rally_display ++){ //Clear the column and make it black. 
			ledmatrix_update_pixel(MATRIX_NUM_COLUMNS-1,board2_rally_display, COLOUR_BLACK);
		}
	}
	
	else {
		player2_display_index = (player2_rally_number % 8) -1;
	}
	
	for (board2_rally_display = 0; board2_rally_display <= player2_display_index; board2_rally_display ++){
		ledmatrix_update_pixel(MATRIX_NUM_COLUMNS-1,board2_rally_display, COLOUR_RALLY_COUNTER);
	}
	
}

void clear_rally2_counter(void){
	player2_rally_number = 0;
	for (int board1_rally_display = 0; board1_rally_display <= 8; board1_rally_display ++){
		ledmatrix_update_pixel(MATRIX_NUM_COLUMNS-1,board1_rally_display, COLOUR_BLACK);
	}
}








// Update ball position based on current x direction and y direction of ball
void update_ball_position(void) {
	


		//Generate a random number to then use to make a randomized x and y direction:
		srand(get_current_time()); //Seed the random number generator.
		
		//Generate arrays with the possible direction factors for y, including for when it is bouncing off the top and bottom corners. 
		int y_direction_array[] = {-1, 0 , 1};
		int y_direction_top_corner_array[] = {0,-1}; 
		int y_direction_bottom_corner_array[] = {0,1}; 
			
		//Generate an array with possible x-direction factors (either 1 or -1): 
		int x_direction_array[] = {-1,1}; 
		
		int random_number_y  = rand() % 3 ; //Selects a random number which is either 0,1,2
		int two_random_numbers = rand() % 2; //Selects a random number which is either 0 or 1.  
	
		//Randomly select numbers from the array and set them to the y and x direction variables: 
		int direction_number_y = y_direction_array [random_number_y] ; //Randomly select an array number from the y_direction_array.
		int top_corner_y = y_direction_top_corner_array[two_random_numbers]; //Selects a value from the top corner array (either 0 or -1)
		int bottom_corner_y = y_direction_bottom_corner_array[two_random_numbers]; //Selects a value from the bottom corner array (either 0 or 1) 
		int direction_number_x  = x_direction_array[two_random_numbers]; 
		
	
		//Write conditional statements for when the ball hits the top and bottom wall.
		if (ball_y == BOARD_HEIGHT-1 || ball_y == 0)
		{
			ball_y_direction = (-1)* ball_y_direction;
		}
		
		//For approaching player 1:	
		if (ball_x + ball_x_direction  == PLAYER_1_X){		//Declare and if statement for when the ball is approaching player_1 paddle axis.

			
			//Invert ball_x direction if the next y_position will be on paddle one
			if (ball_y + ball_y_direction == player_y_coordinates[PLAYER_1]  || ball_y  + ball_y_direction== player_y_coordinates[PLAYER_1]+1){
				ball_x_direction = ball_x_direction* (-1);
				player1_rally_number += 1; //Increment the player rally number. 
				player_1_raller_counter(); //Call the player_1_rally_counter function to display the rally score on the side. 
				
				//Write nested if loops to determine the y-direction of the ball when hitting the paddle. 
				if (ball_y == BOARD_HEIGHT -1){  //To determine ball movement at the top of the board. 
					ball_y_direction = top_corner_y; 
					}
				
				else if (ball_y == 0){   //To determine ball movement at the bottom of the board. 
					ball_y_direction = bottom_corner_y; 
					}
				else {
					ball_y_direction = direction_number_y; 
					}
			}
		}
		
		//For approaching player 2:
		if (ball_x + ball_x_direction == PLAYER_2_X){
			
			//Change he ball_x direction if the next y_position of the ball will be on paddle 2.
			if (ball_y + ball_y_direction == player_y_coordinates[PLAYER_2]  || ball_y  + ball_y_direction== player_y_coordinates[PLAYER_2]+1){
				ball_x_direction = ball_x_direction* (-1);
				player2_rally_number += 1; 
				player_2_raller_counter();
				
				//Write conditional statements for the ball bouncing off paddle 2 when it is at the corners. These 3 conditional statements control the y-direction for the ball when bouncing of the paddle. 
				if (ball_y == BOARD_HEIGHT -1){  //To determine ball movement at the top of the board.
					ball_y_direction =  top_corner_y;
					}
					
				else if (ball_y == 0){   //To determine ball movement at the bottom of the board.
					ball_y_direction = bottom_corner_y;
					}
				else {
					ball_y_direction =  direction_number_y;
					}
				}
		}


	// Determine new ball coordinates
	int8_t new_ball_x = ball_x + ball_x_direction;
	int8_t new_ball_y = ball_y + ball_y_direction;

	/*Write conditional statement for when the ball comes into contact with the side walls (is one spot to the right or left of the side wall),
	not when it actually goes onto the side wall (shares the same x-coordinate of the wall)*/
	if (ball_x == PLAYER_1_X || ball_x ==  PLAYER_2_X ){
		ball_y_direction = direction_number_y;
		ball_x_direction = direction_number_x; 
		new_ball_x = BALL_START_X;
		new_ball_y = BALL_START_Y;
		}
	
	//Code to increment player points: 
	//Point increment for player 2:
	if (ball_x == PLAYER_1_X){
		player2_score += 1; 
		show_cursor();
		move_terminal_cursor(10,14);
		printf("Player 2 Score: %d", player2_score);
		clear_rally1_counter();
		clear_rally2_counter(); 

		}
	
	//Point increment for player 1:
	if (ball_x == PLAYER_2_X){
		player1_score +=1;
		show_cursor();
		move_terminal_cursor(10,13);
		printf("Player 1 Score: %d", player1_score);
		clear_rally1_counter();
		clear_rally2_counter();
	}
	
	//To connect to seven segment display. 

	// Erase old ball
	update_square_colour(ball_x, ball_y, EMPTY_SQUARE);
	
	// Assign new ball coordinates
	ball_x = new_ball_x;
	ball_y = new_ball_y;
	
	//Assign random ball direction: 
	
	// Draw new ball
	update_square_colour(ball_x, ball_y, BALL);
	

}



// Returns 1 if the game is over, 0 otherwise.
uint8_t is_game_over(void) {

	if (player1_score == 3 || player2_score == 3){
		return 1;
		player1_score = 0;
		player2_score = 0;
		player1_rally_number = 0;
		player2_rally_number = 0; 
	}
	
	else {
		return 0;
		
	}
}

	
