#include <iostream>
#include <string>

using namespace std;

int main(){
	std::string path = "/home/pi/Desktop/CameramanJohn/faces/orl_faces/s";
	for(int i = 0; i < 40; i++){
		for(int j = 1; j <= 10; j++){
			cout << path;
			printf("%i/%i.pgm;%i\n", (i+1), j, i);
		}
	}
	for(int i = 0; i <= 121; i++){
		cout << path;
		printf("41/kevin%i.pgm;Kevin\n", i);
	}
}
