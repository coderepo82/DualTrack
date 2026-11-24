#include "./DualTrack.cpp"
#include <iostream>

using namespace std;

int main(int argc, char** argv) {
	for (int i = 1; i <= 10000; i ++) {
		for (int j = 1; j <= 10000; j++) {
			update(i, j);
		}
	}
    return 0;
}
