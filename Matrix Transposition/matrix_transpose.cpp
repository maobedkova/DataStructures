#include <iostream>
#include <cmath>
#include <chrono>

using namespace std;

int *matrix;
int matrix_size;
int overall_swaps;

inline void trivial_algorithm(int x0, int y0, int xn, int yn, bool mode) {
    for (int x = x0; x <= xn; x++) {
        for (int y = x + 1; y <= yn; y++) {
            if (mode) {
                cout << "X " << x << " " << y << " " << y << " " << x << endl;
            } else {
                int r = matrix_size * y + x;
                int c = matrix_size * x + y;
                long elm = matrix[r];
                matrix[r] = matrix[c];
                matrix[c] = elm;
                overall_swaps += 1;
            }
        }
    }
}

inline void print(){
    for (int i = 0; i < matrix_size * matrix_size; ++i) {
        if (i % matrix_size == 0){
            cout << endl;
        }
        cout << matrix[i] << " ";
    }
}

// transpose and swap for cache-oblivious algorithm
inline void transpose(int x0, int y0, int xn, int yn, bool mode){
    for(int x1 = x0; x1 <= xn; x1++){
        int i = y0;
        for(int y1 = y0; y1 <= yn; y1++){
            if (mode){
                cout << "X " << x1 << " " << y1 << " " << i << " " << x0 << endl;
            }
            else{
                int r = matrix_size * y1 + x1;
                int c = matrix_size * x0 + i;
                long elm = matrix[r];
                matrix[r] = matrix[c];
                matrix[c] = elm;
                ++overall_swaps;
            }
            i++;
        }
        x0++;
    }
}

inline void transpose_and_swap(int x0, int y0, int xn, int yn, bool mode){
    if ((xn - x0) < 2 and (yn - y0) < 2) {
        transpose(x0, y0, xn, yn, mode);
    }
    else {
        double fx = floor(double(xn + x0) / 2);
        double fy = floor(double(yn + y0) / 2);
        double cx = fx + 1; 
		double cy = fy + 1;
		
        // A submatrix
        transpose_and_swap(x0, y0, fx, fy, mode);
        // D submatrix
        transpose_and_swap(cx, cy, xn, yn, mode);
        // C submatrix
        transpose_and_swap(x0, cy, fx, yn, mode);
        // B submatrix
        transpose_and_swap(cx, y0, xn, fy, mode);
    }
}

inline void recursive_algorithm(int x0, int y0, int xn, int yn, bool mode){
    if ((xn - x0) < 2 and (yn - y0) < 2){
        trivial_algorithm(x0, y0, xn, yn, mode);
    }
    else if ((xn - x0) < 1 or (yn - y0) < 1){
        return ;
    }
    else{
        double fx = floor(double(xn + x0) / 2);
        double fy = floor(double(yn + y0) / 2);
        double cx = fx + 1;
		double cy = fy + 1;

        // A submatrix
        recursive_algorithm(x0, y0, fx, fy, mode);
        // D submatrix
        recursive_algorithm(cx, cy, xn, yn, mode);
        // B and C submatrices
        transpose_and_swap(x0, cy, fx, yn, mode);
    }
}

int main(int argc, char *argv[] ) {
    int mode = atoi(argv[1]);
	
	/// mode = {1, 2, 3, 4}
    /// 1: Measure running time fot trivial algorithm
    /// 2: Measure running time for cache-oblivious algorithm
    /// 3: Measure page faults with cache simulator for trivial algorithm
    /// 4: Measure page faults with cache simulator for cache-oblivious algorithm

	// matrix sizes
    for(int k = 54; k < 150; ++k) {
        matrix_size = ceil(pow(2, k / 9.0));

		// measure time for trivial and cache-oblivious
		if (mode == 1 or mode == 2) {
			matrix = new int[matrix_size * matrix_size];

			//matrix initialization
			for (int i = 0; i < matrix_size * matrix_size; ++i) {
				matrix[i] = i;
			}

			float time_per_matrix = 0;

			// iteration for measuring time
			int iters = 20;
				while (iters > 0) {
				overall_swaps = 0;
				float overall_dur = 0;

				auto begin = std::chrono::high_resolution_clock::now();
				if (mode == 1) {
					trivial_algorithm(0, 0, matrix_size - 1, matrix_size - 1, false);
				}
				else{
					recursive_algorithm(0, 0, matrix_size - 1, matrix_size - 1, false);
				}
				auto end = std::chrono::high_resolution_clock::now();
				overall_dur = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count();

				time_per_matrix += overall_dur / overall_swaps;
				iters--;
				}
				
			//print();

			// time per swap for matrix_size
			cout << matrix_size << " " << time_per_matrix / 20 << endl;

			delete[] matrix;
		}
		// measure page faults for trivial and cache-oblivious
		else{
			cout << "N " << matrix_size << endl;
			if (mode == 3){
				trivial_algorithm(0, 0, matrix_size - 1, matrix_size - 1, true);
			}
			else{
				recursive_algorithm(0, 0, matrix_size - 1, matrix_size - 1, true);
			}
			cout << "E" << endl;
		}
    }

    return 0;
}