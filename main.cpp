// PROJECT IDENTIFIER: AD48FB4835AF347EB0CA8009E24C3B13F8519882
#include <iostream>
#include <vector>
#include <stack>
#include <queue>
#include <fstream>
#include <string>
#include <sstream>
#include <getopt.h>
#include "P2random.h"


using namespace std;

uint32_t numGen = 0;
uint32_t numPlanets = 0;
uint32_t seed = 0;
uint32_t deployments = 0;
uint32_t rate = 0;
uint32_t planet = 0;
uint32_t numBattles = 0;
uint32_t maxTime = 0;
uint32_t timeOfAttack = 0;
uint32_t timeVar = 0;

bool jediFirst = false;

bool median = false;
bool gen_eval = false;
bool watcher = false;
bool verbose = false;

enum class State {Initial, SeenOne, SeenBoth, MaybeBetter};
  
void get_options(int argc, char** argv){
        int option_index = 0, option = 0;
    
        
    opterr = false;
        
    struct option longOpts[] = {{"median", no_argument, nullptr, 'm'},
                                {"general-eval", no_argument, nullptr, 'g'},
                                {"watcher", no_argument, nullptr, 'w'},
                                {"verbose", no_argument, nullptr, 'v'},
                                {nullptr, 0, nullptr, '\0'}};
    
    while ((option = getopt_long(argc, argv, "mgwv", longOpts, &option_index)) != -1) {
        switch (option) {
            case 'm':
                median = true;
                break;

            case 'g':
                gen_eval = true;
                break;   

            case 'w':    
                watcher = true;
                break;

            case 'v':
                verbose = true;
                break;

            default:
                exit(1);

        }  
    }
}

class RunningMedian {
 private:

  priority_queue<uint32_t, vector<uint32_t>, greater<uint32_t> > upper;

  priority_queue<uint32_t, vector<uint32_t>, less<uint32_t> > lower;

 public:
  RunningMedian(){
    upper.push(4294967295);
    lower.push (0);
  }
  void push(uint32_t val){

    if(val >= upper.top()){
      upper.push(val);
    }
    else {
      lower.push(val);
    }
    if(upper.size() - lower.size() == 2){ 
      lower.push(upper.top());        
      upper.pop();                    
    } else if(lower.size() - upper.size() == 2){ 
      upper.push(lower.top());               
      lower.pop();                           
    }
  }
    uint32_t getMedian() const {
    if(upper.size() == lower.size())               
      return(upper.top() + lower.top()) / ((uint32_t)2.0);  
    else if(upper.size() > lower.size())           
      return upper.top();
    else                                         
      return lower.top();
  }
    bool empty(){
        if(upper.size() == 1 && lower.size() == 1){
            return true;
        }   else {
            return false;
        }
    }
  

};

struct evalMode{
    uint32_t jediCount = 0;
    uint32_t sithCount = 0;
    uint32_t survived = 0;
};

vector <evalMode> generalsList;

vector <RunningMedian> calculateMedian;


struct deploy{
    uint32_t time = 0;
    uint32_t general = 0;
    uint32_t force = 0;
    uint32_t troops = 0;
    uint32_t uniqname = 0;

};

struct attackStruct{
    
    char condition = 'i';
    int maxSithTime = -1;
    
    uint32_t minJediTime = 0;
    uint32_t minJediForce = 0;
    uint32_t maxSithForce = 0;
    
    uint32_t minJediForce2 = 0;
    uint32_t minJediTime2 = 0;

};

struct ambushStruct{
    
    char condition = 'i';
    int minJediTime = -1;
    
    
    uint32_t maxSithTime = 0;
    uint32_t minJediForce = 0;
    uint32_t maxSithForce = 0;
    
    uint32_t maxSithForce2 = 0;
    uint32_t maxSithTime2 = 0;
};

vector <ambushStruct> ambushVector;
vector <attackStruct> attackVector;

class compareSith{
    public:
    bool operator()(const deploy &a, const deploy &b){
        if(a.force < b.force){
        return true;
        } else if(a.force > b.force){
        return false;
        } else {
            return a.uniqname > b.uniqname;
        }
    }

};

class compareJedi{
    public:
    bool operator()(const deploy &a, const deploy &b){
    if(a.force > b.force){
    return true;
    } else if(a.force < b.force){
    return false;
    } else {
    return a.uniqname > b.uniqname;
    }
    }
};

struct planetPQ{

priority_queue<deploy, vector<deploy>, compareSith> sithPQ;
priority_queue<deploy, vector<deploy>, compareJedi> jediPQ;

};

void medianMode(uint32_t time){
        for(uint32_t i = 0; i < numPlanets; i++){
            if(!calculateMedian[i].empty()){
        cout << "Median troops lost on planet " << i << " at time " << time << " is " << calculateMedian[i].getMedian() << ".\n";
            }
        } 
}

void ambush(uint32_t planetNumber, uint32_t timeStamp, uint32_t forceSense, char side){
    ambushStruct &alpha = ambushVector[planetNumber];
    if(alpha.condition == 'b') {
        if(side == 's') {
            if(forceSense > alpha.maxSithForce2) {
                alpha.maxSithForce2 = forceSense;
                alpha.maxSithTime2 = timeStamp;
            } 
        }
        else { 
            if(alpha.maxSithForce2 > forceSense && (alpha.maxSithForce2 - forceSense > (alpha.maxSithForce - alpha.minJediForce))) { 
                alpha.maxSithForce = alpha.maxSithForce2;
                alpha.minJediTime = static_cast<int>(timeStamp);
                alpha.maxSithTime = alpha.maxSithTime2;     
                alpha.minJediForce = forceSense;
            }
        }
    }
    else if(alpha.condition == 's') { 
        if(side == 's') { 
            if(forceSense > alpha.maxSithForce2) {
                alpha.maxSithTime = timeStamp;
                alpha.maxSithTime2 = timeStamp;
                alpha.maxSithForce = forceSense;
                alpha.maxSithForce2 = forceSense;
            } 
        }
        else { 
            if(forceSense <= alpha.maxSithForce2) {
                alpha.condition = 'b';
                alpha.minJediForce = forceSense;
                alpha.minJediTime = static_cast<int>(timeStamp);
            }
        }
    }
    else if(alpha.condition == 'i') { 
        if(side == 's') {
            alpha.maxSithForce = forceSense;
            alpha.maxSithForce2 = forceSense;
            alpha.condition = 's';
            alpha.maxSithTime = timeStamp;
            alpha.maxSithTime2 = timeStamp;
        } 
    }
}



void attack(uint32_t planetNumber, uint32_t timeStamp, uint32_t forceSense, char side){
    attackStruct &alpha = attackVector[planetNumber];
    if(alpha.condition == 'b') {
        if(side == 'j') {
            if(forceSense < alpha.minJediForce2) {
                alpha.minJediForce2 = forceSense;
                alpha.minJediTime2 = timeStamp;
            } 
        }
        else { 
            if(forceSense > alpha.minJediForce2 && (forceSense - alpha.minJediForce2 > (alpha.maxSithForce - alpha.minJediForce))) { 
                alpha.maxSithForce = forceSense;
                alpha.minJediForce = alpha.minJediForce2;
                alpha.maxSithTime = static_cast<int>(timeStamp);
                alpha.minJediTime = alpha.minJediTime2;
            }
        }
    }
    else if(alpha.condition == 'j') { 
        if(side == 'j') { 
            if(forceSense < alpha.minJediForce2) {
                alpha.minJediForce = forceSense;
                alpha.minJediForce2 = forceSense;
                alpha.minJediTime = timeStamp;
                alpha.minJediTime2 = timeStamp;
            } 
        }
        else { 
            if(forceSense >= alpha.minJediForce2) {
                alpha.condition = 'b';
                alpha.maxSithForce = forceSense;
                alpha.maxSithTime = static_cast<int>(timeStamp);
            }
        }
    }
    else if(alpha.condition == 'i') { 
        if(side == 'j') {
            alpha.minJediForce = forceSense;
            alpha.minJediForce2 = forceSense;
            alpha.condition = 'j';
            alpha.minJediTime = timeStamp;
            alpha.minJediTime2 = timeStamp;
        } 
    }
}

vector <planetPQ> planetSorted;

void attack(uint32_t alpha){
    uint32_t troopsRemaining = 0;
    
    deploy survivors;
    
    
    if(planetSorted[alpha].sithPQ.top().troops > planetSorted[alpha].jediPQ.top().troops){ //for when sith have more troops
        
        troopsRemaining = planetSorted[alpha].sithPQ.top().troops - planetSorted[alpha].jediPQ.top().troops;
        
        survivors.time = planetSorted[alpha].sithPQ.top().time;
        survivors.general = planetSorted[alpha].sithPQ.top().general;
        survivors.force = planetSorted[alpha].sithPQ.top().force;
        survivors.troops = troopsRemaining;
        
        
        calculateMedian[alpha].push(planetSorted[alpha].jediPQ.top().troops * 2);
        

        if(verbose){
            cout << "General " << planetSorted[alpha].sithPQ.top().general << "'s battalion attacked General " << planetSorted[alpha].jediPQ.top().general << "'s battalion on planet " << alpha << ". " << planetSorted[alpha].jediPQ.top().troops * 2 << " troops were lost.\n";
        }

        
        generalsList[planetSorted[alpha].jediPQ.top().general].survived += planetSorted[alpha].jediPQ.top().troops;
        generalsList[planetSorted[alpha].sithPQ.top().general].survived += planetSorted[alpha].jediPQ.top().troops;

        planetSorted[alpha].sithPQ.pop();
        planetSorted[alpha].jediPQ.pop();
        planetSorted[alpha].sithPQ.push(survivors);
        
        return;

    }
    else if(planetSorted[alpha].sithPQ.top().troops < planetSorted[alpha].jediPQ.top().troops){
        

        
        troopsRemaining = planetSorted[alpha].jediPQ.top().troops - planetSorted[alpha].sithPQ.top().troops;
        survivors.time = planetSorted[alpha].jediPQ.top().time;
        survivors.general = planetSorted[alpha].jediPQ.top().general;
        survivors.force = planetSorted[alpha].jediPQ.top().force;
        survivors.troops = troopsRemaining;

        
        calculateMedian[alpha].push(planetSorted[alpha].sithPQ.top().troops * 2);
        

        if(verbose){
            cout << "General " << planetSorted[alpha].sithPQ.top().general << "'s battalion attacked General " << planetSorted[alpha].jediPQ.top().general;
            cout << "'s battalion on planet " << alpha << ". " << planetSorted[alpha].sithPQ.top().troops * 2 << " troops were lost.\n"; 
        }


        generalsList[planetSorted[alpha].jediPQ.top().general].survived += planetSorted[alpha].sithPQ.top().troops;
        generalsList[planetSorted[alpha].sithPQ.top().general].survived += planetSorted[alpha].sithPQ.top().troops;

        planetSorted[alpha].sithPQ.pop();
        planetSorted[alpha].jediPQ.pop();
        planetSorted[alpha].jediPQ.push(survivors);
        
        return;
    } else{

        
        calculateMedian[alpha].push( planetSorted[alpha].jediPQ.top().troops * 2);
        

        if(verbose){
            cout << "General " << planetSorted[alpha].sithPQ.top().general << "'s battalion attacked General " << planetSorted[alpha].jediPQ.top().general;
            cout  << "'s battalion on planet " << alpha << ". " << planetSorted[alpha].jediPQ.top().troops * 2 << " troops were lost.\n";
        }

        generalsList[planetSorted[alpha].jediPQ.top().general].survived += planetSorted[alpha].jediPQ.top().troops;
        generalsList[planetSorted[alpha].sithPQ.top().general].survived += planetSorted[alpha].jediPQ.top().troops; 

        planetSorted[alpha].jediPQ.pop();
        planetSorted[alpha].sithPQ.pop();
        
        return;
    }
}



void checkValid(uint32_t planetIndex) {
    
    if(planetSorted[planetIndex].jediPQ.empty() || planetSorted[planetIndex].sithPQ.empty()){

        return;
    }
    if(planetSorted[planetIndex].sithPQ.top().force < planetSorted[planetIndex].jediPQ.top().force){

        return;
    }
    while((!planetSorted[planetIndex].sithPQ.empty() && !planetSorted[planetIndex].jediPQ.empty()) && planetSorted[planetIndex].sithPQ.top().force >= planetSorted[planetIndex].jediPQ.top().force){ 
        if(jediFirst){
            timeOfAttack = planetSorted[planetIndex].jediPQ.top().time;
        } else {
            timeOfAttack = planetSorted[planetIndex].sithPQ.top().time;
        }
        
        attack(planetIndex);    
        numBattles++;  

    }
    
    return;
}
    

void readFile() {
    
    string inputMode;
    stringstream ss;
    string junk;
    char sideHolder;
    uint32_t prevTime = 0;
    char junk2;
    bool isJedi = false;
    
    deploy orders; 
    uint32_t incid = 0; 

    getline(cin,junk); // junk is now holding first comment line
    cin >> junk; //junk is now holding "MODE:"
    cin >> inputMode;
    cin >> junk;
    cin >> numGen;
    cin >> junk;
    cin >> numPlanets;

    planetSorted.resize(numPlanets);
    generalsList.resize(numGen);
    calculateMedian.resize(numPlanets);
    ambushVector.resize(numPlanets);
    attackVector.resize(numPlanets);
    

    if(inputMode == "PR"){
        cin >> junk;
        cin >> seed;
        cin >> junk;
        cin >> deployments;
        cin >> junk;
        cin >> rate;
    P2random::PR_init(ss, seed, numGen, numPlanets, deployments, rate);

        while(ss >> orders.time){ 
        
        maxTime = orders.time;
    
        if(orders.time < prevTime){
            cerr << "Time cannot decrease" << endl;
            exit(1);
        }

        if(orders.time != prevTime){        
                if(median){
                    medianMode(prevTime);
                }
                prevTime = orders.time;
            
        }
        

        ss >> junk;
        
        if(junk == "JEDI"){
            isJedi = true;
        } else {
            isJedi = false;
        }

        
        ss >> junk2;
        ss >> orders.general;

        if(orders.general >= numGen){
            cerr << "General ID exceeds numGen" << endl;
            exit(1);
        }

        ss >> junk2;
        ss >> planet;

        if(planet >= numPlanets){
            cerr << "Number of planets exceeds numPlanets" << endl;
            exit(1);
        }

        ss >> junk2;
        ss >> orders.force;

        if(orders.force <= 0){
            cerr << "Force has to be greater than 0" << endl;
            exit(1);
        }

        ss >> junk2;
        ss >> orders.troops;

        if(orders.troops <= 0){
            cerr << "Troops has to be greater than 0" << endl;
            exit(1);
        }

        orders.uniqname = incid;
        incid++;

        if(isJedi){
            sideHolder = 'j';
            generalsList[orders.general].jediCount += orders.troops;
            planetSorted[planet].jediPQ.push(orders);
            jediFirst = true;
            checkValid(planet);
        } else {
            sideHolder = 's';
            generalsList[orders.general].sithCount += orders.troops;
            planetSorted[planet].sithPQ.push(orders);
            checkValid(planet);
        }
        jediFirst = false;
        if(watcher){
            ambush(planet, orders.time, orders.force, sideHolder);
            attack(planet, orders.time, orders.force, sideHolder);
        }
    }
    



    if(median){
        for(uint32_t i = 0; i < numPlanets; i++){
            if(!calculateMedian[i].empty()){
        cout << "Median troops lost on planet " << i << " at time " << timeOfAttack << " is " << calculateMedian[i].getMedian() << ".\n";
            }
        } 
    }


    } else {
    
    while(cin >> orders.time){
        
        maxTime = orders.time;

    if(orders.time < prevTime){
        cerr << "Time cannot decrease" << endl;
        exit(1);
    }

    if(orders.time != prevTime){        
            if(median){
                medianMode(prevTime);
            }
            prevTime = orders.time;
        
    }

        cin >> junk;
        
        if(junk == "JEDI"){
            isJedi = true;
        } else {
            isJedi = false;
        }

        
        cin >> junk2;
        cin >> orders.general;

        if(orders.general >= numGen){
            cerr << "General ID exceeds numGen" << endl;
            exit(1);
        }

        cin >> junk2;
        cin >> planet;
        
        if(planet >= numPlanets){
            cerr << "Number of planets exceeds numPlanets" << endl;
            exit(1);
        }

        cin >> junk2;
        cin >> orders.force;

        if(orders.force <= 0){
            cerr << "Force has to be greater than 0" << endl;
            exit(1);
        }
        
        cin >> junk2;
        cin >> orders.troops;

        if(orders.troops <= 0){
            cerr << "Troops has to be greater than 0" << endl;
            exit(1);
        }
        
        orders.uniqname = incid;
        incid++;

        if(isJedi){
            
            sideHolder = 'j';
            generalsList[orders.general].jediCount += orders.troops;
            planetSorted[planet].jediPQ.push(orders);
            jediFirst = true;
            
            checkValid(planet);
        } else {
            
            sideHolder = 's';
            generalsList[orders.general].sithCount += orders.troops;
            
            planetSorted[planet].sithPQ.push(orders);
            
            checkValid(planet);

        }
        jediFirst = false;

        if(watcher){
            ambush(planet, orders.time, orders.force, sideHolder);
            attack(planet, orders.time, orders.force, sideHolder);
        }
        
    }

    if(median){
        
        for(uint32_t i = 0; i < numPlanets; i++){
            if(!calculateMedian[i].empty()){
        cout << "Median troops lost on planet " << i << " at time " << timeOfAttack << " is " << calculateMedian[i].getMedian() << ".\n";
            }
        }
        
    }
    }
    


}

void standard(){
    cout << "---End of Day---" << '\n';
    cout << "Battles: " << numBattles << '\n';

}

void generalEval(){
    cout << "---General Evaluation---" << '\n';
    for(uint32_t i = 0; i < numGen; i++){
        cout << "General " << i << " deployed " << generalsList[i].jediCount << " Jedi troops and " << generalsList[i].sithCount << " Sith troops, and " << (generalsList[i].sithCount + generalsList[i].jediCount) - generalsList[i].survived << "/" << generalsList[i].sithCount + generalsList[i].jediCount << " troops survived." << '\n';
    }
}

void movieWatcher(){
    cout << "---Movie Watcher---" << '\n';
    for(size_t i = 0; i < numPlanets; ++i) {
        if(ambushVector[i].minJediTime != -1) {
            cout << "A movie watcher would enjoy an ambush on planet " << i << " with Sith at time " << ambushVector[i].maxSithTime << " and Jedi at time " << ambushVector[i].minJediTime << " with a force difference of " << ambushVector[i].maxSithForce - ambushVector[i].minJediForce << ".\n";
        } else {
            cout << "A movie watcher would enjoy an ambush on planet " << i << " with Sith at time -1 and Jedi at time -1 with a force difference of 0.\n"; 
        }
        if(attackVector[i].maxSithTime != -1) {
            cout << "A movie watcher would enjoy an attack on planet " << i << " with Jedi at time " << attackVector[i].minJediTime << " and Sith at time " << attackVector[i].maxSithTime << " with a force difference of " << attackVector[i].maxSithForce - attackVector[i].minJediForce << ".\n";
        } else {
            cout << "A movie watcher would enjoy an attack on planet " << i << " with Jedi at time -1 and Sith at time -1 with a force difference of 0.\n"; 
        }

    }


}



int main(int argc, char** argv){
        ios_base::sync_with_stdio(false);
        int option_index = 0, option = 0;
    
        
        opterr = false;
        
        struct option longOpts[] = {{"median", no_argument, nullptr, 'm'},
                                    {"general-eval", no_argument, nullptr, 'g'},
                                    {"watcher", no_argument, nullptr, 'w'},
                                    {"verbose", no_argument, nullptr, 'v'},
                                    {nullptr, 0, nullptr, '\0'}};
    
        while ((option = getopt_long(argc, argv, "mgwv", longOpts, &option_index)) != -1) {
            switch (option) {
                case 'm':
                    median = true;
                    break;

                case 'g':
                    gen_eval = true;
                    break;   

                case 'w':    
                    watcher = true;
                    break;

                case 'v':
                    verbose = true;
                    break;

                default:
                    exit(1);

            }  
        }

    cout << "Deploying troops..." << endl;
    
    
    readFile();
    
    standard();

    if(gen_eval){
    generalEval();
    }

    if(watcher){
    movieWatcher();
    }
    

    }














