#include <iostream>
#include <vector>
#include <time.h>
#include <map>
#include <fstream>
using namespace std;

using ve = vector<int>;
using vs = vector<string>;


int P; //number of films
int L; //number of film pairs that can NOT be projected at the same time.
int S; //number of cinemas
int solution;
clock_t start;
vector<ve> restrictions;
vs films; 
map<string, int> films_map;
vs cinemas;

/*
Function that reads data from an .inp file with certain specifications 
and fills the vectors and variables as requeired.
*/
void read_instance(const char* file) {

	ifstream in(file);

	in >> P;
	solution = P+1;
	films.resize(P); 
	for (int k = 0; k < P; ++k) {
    	in >> films[k]; 
    	films_map[ films[k] ] = k; 
	}

	in >> L;
	restrictions = vector<ve>(P, ve(P, -1));
	for(int i = 0; i < L; ++i){
		string film1, film2;
		in >> film1 >> film2;
		int p = films_map[film1];
		int q = films_map[film2];
		restrictions[p][q] = 1;
		restrictions[q][p] = 1;
	}

  	in >> S;
  	cinemas.resize(S); 
  	for (int k = 0; k < S; ++k) in >> cinemas[k];
}

/*
Function that overwrites an .inp file with the seconds needed to find the solution, 
the days that the festival lasts and a list with the title of the film next to the day
and the cinema to which they have been assigned.
*/
void write_solution(const char* file, vector<ve>& schedule){
	
	ofstream out(file);
	out.setf(ios::fixed);
   	out.precision(1);

	clock_t end;
	end = clock();
	double time_taken = ((double)(end - start))/CLOCKS_PER_SEC;
	out << time_taken << endl;

	out << solution << endl;
	for(int i = 0; i < solution ; ++i){
		int n_cinema = 0;
		for(int j : schedule[i]){
			out << films[j] << " " << i+1 << " " << cinemas[n_cinema] << endl;
			++n_cinema;
		}
	}
	out.close();
}

/*
Boolean function that returns false in case the given movie has restrictions within a certain day. 
It returns true otherwise.
*/
bool allowed(int p, ve& schedule){
	
	for(int f : schedule){
		if(restrictions[p][f] == 1) return false;
	} return true;
}

/*
Exhaustive search function that every time you get a better solution
for the proposed problem, overwrites it in the output file.
It is a recursive function that in each iteration tries to place the k-th film into the given schedule.
In case the film does not fit in the schedule, a new day is added to host it.
*/
void film_festival(int k, vector<ve>& schedule, const char* file){
	
	if(schedule.size() >= solution) return;

	//If all movies have been assigned, we compare the result.
	if(k == P){
		solution = schedule.size();
		write_solution(file, schedule);
		return;
	}

	for(int d = 0; d <= schedule.size(); ++d){
		if(d == schedule.size()){
			schedule.push_back(vector<int>(1, k)); 
			film_festival(k+1, schedule, file);
            schedule.pop_back();
		}
		else if(allowed(k, schedule[d]) and schedule[d].size() < S){
			schedule[d].push_back(k);
			film_festival(k+1, schedule, file);
			schedule[d].pop_back();
		} 			
	}
}	

int main(int argc, char** argv){
    start = clock();
	read_instance(argv[1]);
	vector<ve> schedule; 
	film_festival(0, schedule, argv[2]);
}
