#include <wiringPi.h>
#include <stdio.h>

int main(void)
{
		wiringPiSetup();

		//Sets up the control pins
		int controlPin[4] = {0,1,2,3};
		pinMode(controlPin[0], OUTPUT);
		pinMode(controlPin[1], OUTPUT);
		pinMode(controlPin[2], OUTPUT);
		pinMode(controlPin[3], OUTPUT);

		//sequence for half step
		int seqR[8][4]= {{1,0,0,0},
				{1,1,0,0},
				{0,1,0,0},
				{0,1,1,0},
				{0,0,1,0},
				{0,0,1,1},
				{0,0,0,1},
				{1,0,0,1}};
		int seqL[8][4]= {{0,0,0,1},
				{0,0,1,1},
				{0,0,1,0},
				{0,1,1,0},
				{0,1,0,0},
				{1,1,0,0},
				{1,0,0,0},
				{1,0,0,1}};
		for(int i = 0; i < 512; i++){
				for(int half = 0; half < 8; half++){
						for(int pin = 0; pin < 4; pin++){
								digitalWrite(controlPin[pin], seqL[half][pin]);
						}
						delay(1);
				}
		}
		for(int i = 0; i < 512; i++){
				for(int half = 0; half < 8; half++){
						for(int pin = 0; pin < 4; pin++){
								digitalWrite(controlPin[pin], seqR[half][pin]);
						}
						delay(1);
				}
		}

		return 0;
}
