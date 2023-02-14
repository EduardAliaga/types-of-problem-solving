#include <iostream>
#include <vector>
#include <time.h>
#include <map>
#include <fstream>
#include <algorithm>
#include <math.h>
using namespace std;

using ve = vector<int>;
using vs = vector<string>;

int P; //number of films
int L; //number of film pairs that can NOT be projected at the same time
int S; //number of cinemas
int solution;
clock_t start;
vector<ve> restrictions;
vs films; 
map<string, int> films_map;
vs cinemas;

struct Timetable {
	vector<ve> ttable;
	int days;
	int incomp;
};

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
Function that returns a new "Timetable struct" which corresponds to a random schedule with zero incompatibilities 
and that lasts for j days when all the P-movies have been assigned. 
*/
Timetable generate_initial_solution(){
	vector<ve> s;
	ve p(P);
	for(int k = 0; k < P; ++k){
		p[k] = k;
	}

	random_shuffle(p.begin(), p.end());
	s.push_back(ve(1, p[0]));
	for(int i = 1; i < P; ++i){
		int d = 0;
		bool assigned = false;
		while(not assigned){
			if(allowed(p[i], s[d]) and s[d].size() < S){
				s[d].push_back(p[i]);
				assigned = true;
			} else {
				++d;
				if(d == s.size()){
					s.push_back(vector<int>(1, p[i]));
					assigned = true;
				}
			
			}
		}
		
	}
	int j = s.size();
	Timetable new_schedule = {s, j, 0};
	return new_schedule;
}

/* 
Function that returns the probability of accepting a worsening move. 
It is computed with the Boltzmann distribution.
Given two diferent schedules, since days of schedule2 is always bigger than days of schedule1,
the lower the temperature T the lower the probability of accepting a worsening move.
*/
double probability(double T, int schedule1, int schedule2){
	return exp(-(schedule2-schedule1)/T);
}

/*
Boolean function that returns false if there is any day of the schedule with some day not completed and true otherwise.
*/
bool full(const vector<ve>& schedule){
	for(int i = 0; i < schedule.size(); ++i){
		if(schedule[i].size() < S) return false;
	}	
	return true;
}

/*
Function that returns a new "Timetable struct" which corresponds to a neighbour of the current schedule.
This new schedule could have incompatibilities (just one more or one less than the previous schedule) 
and is created by deleting a film of the day with smaller size and then inserting it on a random day.  
*/
Timetable pick_at_random(const Timetable& current_schedule){
	Timetable schedule = current_schedule;
	int d = P+1;
	int size_d = S+1;
	int n = schedule.days;

	for (int i = 0; i < n; ++i) {
		if(schedule.ttable[i].size() < size_d){
			size_d = schedule.ttable[i].size();
			d = i;
		}
	}
	int film_x = schedule.ttable[d].back();
	schedule.ttable[d].pop_back();

	//If some film that was not allowed with the film that has been released is found inside the d-day of the schedule, the number of incompatibilities decrease.
	if(not allowed(film_x, schedule.ttable[d])) schedule.incomp -= 1;

	//One day is erased from the schedule in case it has been left empty. The number of days of the timetable decerease.
	if(schedule.ttable[d].empty()){
		schedule.ttable[d].swap(schedule.ttable[n-1]);
		schedule.ttable.pop_back();
		schedule.days -= 1;
	} 

	//The film that has been deleted is inserted in a new random day. If it is not allowed we increase the number of incompatibilities.
	//In case the schedule is full, the last film of a randomly selected day is deleted in order to make room for the other film.
	//Then the second deleted movie is inserted on a new day and the days of the schedule increase.
	//Finally the incompatibilities are updated regarding the modifications.
	bool assigned = false;
	while(not assigned){
		int r = rand()%schedule.days;
		if(schedule.ttable[r].size() < S){
			if(not allowed(film_x, schedule.ttable[r])) schedule.incomp += 1;
			schedule.ttable[r].push_back(film_x);
			assigned = true;
		} else if(full(schedule.ttable)){
			int film_y = schedule.ttable[r].back();
			schedule.ttable[r].pop_back();
			if(not allowed(film_y, schedule.ttable[r])) schedule.incomp -= 1;
			if(not allowed(film_x, schedule.ttable[r])) schedule.incomp += 1;
			schedule.ttable[r].push_back(film_x);
			schedule.ttable.push_back(vector<int>(1, film_y));
			schedule.days += 1;
			assigned = true;
		} 
	}
	return schedule;
}

/*
Function that implements the Simulated Annealing methaeuristic algorithm that allow moves resulting in
solutions of worse quality than the current solution in order to escape from local optima.
This solutions could not be written because they  have incompatibilities.
The probability of doing such a move is decreased during the search regarding an alpha factor.
Due to the "while" loop this iterative function never ends and keeps looking for a better solution infinitely.
*/
void film_festival(const char* file, double alpha){
	int hy = 0;
	while(true){
		Timetable schedule = generate_initial_solution();
		if(schedule.days < solution) {
			solution = schedule.days;
			write_solution(file, schedule.ttable);
		}
		int k = 0;
		double T = 1;
		while(k < 10000){
			Timetable alternative_schedule = pick_at_random(schedule);
			if(alternative_schedule.days < solution){
				schedule = alternative_schedule;
				if(schedule.incomp == 0){
					solution = schedule.days;
					write_solution(file, schedule.ttable);
				}
			} else if(rand()%100 < (probability(T, schedule.days, alternative_schedule.days) * 100)) schedule = alternative_schedule; 
			T *= alpha;
			++k;
		}
	}
}	

int main(int argc, char** argv){
    start = clock();
	read_instance(argv[1]); 
	film_festival(argv[2], 0.9);
}