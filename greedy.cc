#include <iostream>
#include <vector>
#include <time.h>
#include <map>
#include <fstream>
#include <algorithm>
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

struct Degree {
	int k, deg;
};

vector<Degree> vdeg;

/*
Boolean function that orders films with decreasing degree of restrictions.
*/
bool ByDegree(const Degree &a, const Degree &b)
{
    return a.deg > b.deg;
}

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
	
	ve times(P, 0);
	in >> L;
	restrictions = vector<ve>(P, ve(P, -1));
	for(int i = 0; i < L; ++i){
		string film1, film2;
		in >> film1 >> film2;
		int p = films_map[film1];
		int q = films_map[film2];
		restrictions[p][q] = 1;
		restrictions[q][p] = 1;
		times[p] += 1;
		times[q] += 1;
	}

	for(int i = 0; i < P; ++i) vdeg.push_back({i,times[i]});
	sort(vdeg.begin(), vdeg.end(), ByDegree);

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
Function that implements a greedy algorithm in order to find a quick solution even though it cannot be guaranteed that it is optimal.
It places all the films sequentially based on a vector that has been previously ordered taking into account the number of constraints.
In case a film does not fit in the schedule, a new day is added to host it.
When we have placed all the films we proceed to write the solution in a file with a name determined by the execution input.
*/
void film_festival(vector<ve>& schedule, const char* file){
	int d = 0;
	schedule.push_back(vector<int>(1, vdeg[0].k));
	for(int i = 1; i < P; ++i){
		if(allowed(vdeg[i].k, schedule[d]) and schedule[d].size() < S) schedule[d].push_back(vdeg[i].k);
		else {
			++d;
			schedule.push_back(vector<int>(1, vdeg[i].k));
		}
	}
	solution = schedule.size();
	write_solution(file, schedule);
}	

int main(int argc, char** argv){
    start = clock();
	read_instance(argv[1]);
	vector<ve> schedule; 
	film_festival(schedule, argv[2]);
}