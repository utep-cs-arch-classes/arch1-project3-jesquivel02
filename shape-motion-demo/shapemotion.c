/** \file shapemotion.c
 * Modified to hold new code for Project 3
 * Jonathan Esquivel
 * Id: 80474221
 * Project 3 Pong
 */  

#include <msp430.h>
#include <libTimer.h>
#include <lcdutils.h>
#include <lcddraw.h>
#include <p2switches.h>
#include "buzzer.h" // Include buzzer from previous lab to add sound
#include <shape.h>
#include <abCircle.h>



#define GREEN_LED BIT6


//Keep naming convetion of rect10 with rect20
AbRect rect10 = {abRectGetBounds, abRectCheck, {2,14}}; /**< 10x10 right paddle */
AbRect rect20 = {abRectGetBounds, abRectCheck, {2,14}}; /**< 10x10 left paddle */

// AbRArrow rightArrow = {abRArrowGetBounds, abRArrowCheck, 30}; /**<Unsure if neccessary, will leave here */

AbRectOutline fieldOutline = {	/* playing field */
  abRectOutlineGetBounds, abRectOutlineCheck,   
  {screenWidth/2 , screenHeight/2 - 10}
};

Layer ball = {		/**< Layer with a white ball */
  (AbShape *)&circle5,
  {(screenWidth/2)+10, (screenHeight/2)+5}, /**< bit below & right of center */
  {0,0}, {0,0},				    /* last & next pos */
  COLOR_WHITE,
  0,
};


Layer field = {		/* playing field as a layer */
  (AbShape *) &fieldOutline,
  {screenWidth/2, screenHeight/2},/**< center */
  {0,0}, {0,0},				    /* last & next pos */
  COLOR_BLACK,
  &ball
};

Layer rightPaddle = {		/**< Layer with a white paddle - Is on right */
  (AbShape *)&rect10,
  {screenWidth - 10, screenHeight/2}, /**< center */
  {0,0}, {0,0},				    /* last & next pos */
  COLOR_WHITE,
  &fieldLayer,
};

Layer leftPaddle = {		/**< Layer with a white paddle - Is on left*/
  (AbShape *)&rect20,
  {(10, (screenHeight/2)}, /**< Move paddle to center */
  {0,0}, {0,0},				    /* last & next pos */
  COLOR_WHITE,
  &rightPaddle,
};

/** Moving Layer
 *  Linked list of layer references
 *  Velocity represents one iteration of change (direction & magnitude)
 */
typedef struct MovLayer_s {
  Layer *layer;
  Vec2 velocity;
  struct MovLayer_s *next;
} MovLayer;

//Change Movlayers here to involve the ball and two paddles
/* initial value of {0,0} will be overwritten */
MovLayer m13 = { &ball, {1,2}, 0 }; /**< not all layers move */
MovLayer m11 = { &rightPaddle, {0,0}, &m13 }; //unsure if names needs to be the way they are, will leave alone
MovLayer m10 = { &leftPaddle, {0,0}, &m11 }; 







movLayerDraw(MovLayer *movLayers, Layer *layers)
{
  int row, col;
  MovLayer *movLayer;

  and_sr(~8);			/**< disable interrupts (GIE off) */
  for (movLayer = movLayers; movLayer; movLayer = movLayer->next) { /* for each moving layer */
    Layer *l = movLayer->layer;
    l->posLast = l->pos;
    l->pos = l->posNext;
  }
  or_sr(8);			/**< disable interrupts (GIE on) */


  for (movLayer = movLayers; movLayer; movLayer = movLayer->next) { /* for each moving layer */
    Region bounds;
    layerGetBounds(movLayer->layer, &bounds);
    lcd_setArea(bounds.topLeft.axes[0], bounds.topLeft.axes[1], 
		bounds.botRight.axes[0], bounds.botRight.axes[1]);
    for (row = bounds.topLeft.axes[1]; row <= bounds.botRight.axes[1]; row++) {
      for (col = bounds.topLeft.axes[0]; col <= bounds.botRight.axes[0]; col++) {
	Vec2 pixelPos = {col, row};
	u_int color = bgColor;
	Layer *probeLayer;
	for (probeLayer = layers; probeLayer; 
	     probeLayer = probeLayer->next) { /* probe all layers, in order */
	  if (abShapeCheck(probeLayer->abShape, &probeLayer->pos, &pixelPos)) {
	    color = probeLayer->color;
	    break; 
	  } /* if probe check */
	} // for checking all layers at col, row
	lcd_writeColor(color); 
      } // for col
    } // for row
  } // for moving layer being updated
}	  



//Region fence = {{10,30}, {SHORT_EDGE_PIXELS-10, LONG_EDGE_PIXELS-10}}; /**< Create a fence region */

/** Advances a moving shape within a fence
 *  
 *  \param m1 The moving shape to be advanced
 *  \param fence The region which will serve as a boundary for m1
 */
void m1Advance(MovLayer *m1, Region *fence)
{
  Vec2 newPos;
  u_char axis;
  Region shapeBoundary;
  for (; m1; m1 = m1->next) {
    vec2Add(&newPos, &m1->layer->posNext, &m1->velocity);
    abShapeGetBounds(m1->layer->abShape, &newPos, &shapeBoundary);
    for (axis = 0; axis < 2; axis ++) {
      if ((shapeBoundary.topLeft.axes[axis] < fence->topLeft.axes[axis]) ||
	  (shapeBoundary.botRight.axes[axis] > fence->botRight.axes[axis]) ) {
	int velocity = m1->velocity.axes[axis] = -m1->velocity.axes[axis];
	newPos.axes[axis] += (2*velocity);
      }	/**< if outside of fence */
    } /**< for axis */
    m1->layer->posNext = newPos;
  } /**< for m1 */
}


u_int bgColor = COLOR_BLACK;     /**< The background color */
int redrawScreen = 1;           /**< Boolean for whether screen needs to be redrawn */

Region fieldFence;		/**< fence around playing field  */


/** Initializes everything, enables interrupts and green LED, 
 *  and handles the rendering for the screen
 */
void main()
{
  P1DIR |= GREEN_LED;		/**< Green led on when CPU on */		
  P1OUT |= GREEN_LED;

  configureClocks();
  lcd_init();
  shapeInit();
  p2sw_init(15);		// Thanks to Jose Perez for explaning how to get switches to work by changing 
				// p2sw_init(x); to  p2sw_init(15);

  shapeInit();

  layerInit(&leftPaddle);
  layerDraw(&leftPaddle);


  layerGetBounds(&fieldLayer, &fieldFence);


  enableWDTInterrupts();      /**< enable periodic interrupt */
  or_sr(0x8);	              /**< GIE (enable interrupts) */


  for(;;) { 
    while (!redrawScreen) { /**< Pause CPU if screen doesn't need updating */
      P1OUT &= ~GREEN_LED;    /**< Green led off witHo CPU */
      movRightPaddle();
      movLeftPaddle();
      resetBall();
      bounceOnPaddle();	    
      or_sr(0x10);	      /**< CPU OFF */
    }
    P1OUT |= GREEN_LED;       /**< Green led on when CPU on */
    redrawScreen = 0;
    movLayerDraw(&m10, &layer0);
  }
}
    //The switches cant be dont seperately,
    //the left paddle needs to be together, and the right paddle needs to be together
    //Thanks to Jose Perez for explaining why they need to be together 
    
//     void movRightPaddleRight(MovLayer* mLayer){
// 	    u_int switches = p2sw_read();
	    
// 	    if(!(switches & (1<<0))){
// 		    mLayer -> velocity.axes[1] = 3;
// 	    }
//     }
    
//         void movRightPaddleLeft(MovLayer* mLayer){
// 	    u_int switches = p2sw_read();
	    
// 	    if(!(switches & (1<<1))){
// 		    mLayer -> velocity.axes[1] = -3;
// 	    }
//     }
    
//         void movLeftPaddleRight(MovLayer* mLayer){
// 	    u_int switches = p2sw_read();
	    
// 	    if(!(switches & (1<<2))){
// 		    mLayer -> velocity.axes[1] = 3;
// 	    }
//     }
    
//         void movLeftPaddleLeft(MovLayer* mLayer){
// 	    u_int switches = p2sw_read();
	    
// 	    if(!(switches & (1<<3))){
// 		    mLayer -> velocity.axes[1] = -3;
// 	    }
// 	}
//  
    
    //movRightPaddle()
    //Will move the right paddle whenever switch 1 or 2 is pressed.
        int movRightPaddle(MovLayer* mLayer){
	    u_int switches = p2sw_read();
	    
		if(!(switches & (1<<0))){		//Use masks to find which switch is being pressed 
		    	mLayer -> velocity.axes[1] = 3;
	    	}
		else if(!(switches & (1<<1))){
			mLayer -> velocity.axes[1] = -3;
		}
		else {
			mLayer -> velocity.axes[1] = 0;
		}
    }
    //movLeftPaddle()
    //Will move the left paddle whenever switch 3 or 4 is pressed
        int movLeftPaddle(MovLayer* mLayer){		
	    u_int switches = p2sw_read();
	   						
		if(!(switches & (1<<2))){		//Use masks to find which switch is being pressed
			mLayer -> velocity.axes[1] = 3;
	    	}	
		else if(!(switches & (1<<3))){
			mLayer -> velocity.axes[1] = -3;
		}
		else
			mLayer -> velocity.axes[1] = 0;
    }
    //resetBall() 
    //Will reset the ball every time the ball either goes too far right, or too far left
    int resetBall(){
	    if(ball.pos.axes[0] > screenWidth){ // If ball has moved off the screen, reset it's speed to go the correct direction
		    m13.velocity.axes[0] = 1;
		    m13.velocity.axes[1] = 2;
	    }
	    else if(ball.pos.axes[0] < 0){ // If ball has moved off the screen, reset it's speed to go the correct direction
		    m13.velocity.axes[0] = -1;
		    m13.velocity.axes[0] = -2;
	    }
	    //Now reset the starting spot of the ball
	    ball.posNext.axes[0] = screenwidth/2;
	    ball.posNext.axes[1] = screenheight/2;
    }
    
    //bouncOnPaddle()
    //Will increase the speed of the ball after every hit on the paddles
    int bounceOnPaddle(){ 
	    //Thanks to Abner Palomino for explaining the way to check ball bouncing off the paddle 
	    //using checkPaddle and bounceOnPaddle
	    
	    //Every time the ball hits one of the paddles, the speed will increase in the x-axis
	    //Every time the ball hits the paddles 3 times, the y-axis speed will increase
	    if(paddleHit()){
		    static char hitCounter = 0;
		    if(m13.velocity.axes[0] > 0){
			    m13.velocity.axes[0] = -(m13.velocity.axes[0]);
			    hitcounter++;
		    }
		    else if(m13.velocity.axes[0] < 0){
			    m13.velocity.axes[0] = -(m13.velocity.axes[0]);
			    hitcounter--;
		    }
		    if((hitcounter % 3) == 0){
			    if(m13.velocity.axes[0] > 0){
			    	m13.veloicty.axes[1]++;
			    }
			    else if(m13.velocity.axes[0] < 0){
			    	m13.velocity.axes[1]--;
			    }
		    }
    }
    int checkPaddle(){
	    Vec2 ballRight = {ball.pos.axes[0]+circle5.radius,ball.pos.axes[1]};
	    Vec2 ballLeft = {ball.pos.axes[0]-circle5.radius,ball.pos.axes[1]};
	    
	    if(abRectCheck(&rect10,&rightPaddle.pos,&ballRight)){
		    return 1;
	    }
	    else if(abRectCheck(&rect20,&leftPaddle.pos,&ballLeft)){
		    return 1;
	    }
	    else{//Unsure if this else is neccessary
		    return 0;
	    }
	    return 0;
    }
    
    
    
/** Watchdog timer interrupt handler. 15 interrupts/sec */
void wdt_c_handler()
{
  static short count = 0;
  P1OUT |= GREEN_LED;		      /**< Green LED on when cpu on */
  count ++;
  if (count == 15) {
    m1Advance(&m10, &fieldFence);
    if (p2sw_read())
      redrawScreen = 1;
    count = 0;
  }
  P1OUT &= ~GREEN_LED;		    /**< Green LED off when cpu off */
}
