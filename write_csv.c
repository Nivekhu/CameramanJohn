#include <iostream>
#include <string>

using namespace std;

int main(){
	std::string path = "/home/pi/Desktop/CameramanJohn/faces/orl_faces/s";
	for(int i = 0; i < 20; i++){
		for(int j = 1; j <= 10; j++){
			cout << path;
			printf("%i/%i.pgm;%i\n", (i+1), j, i);
		}
	}
}
