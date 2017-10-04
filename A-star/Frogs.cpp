//
//  CIS479
//
//  Created by Ali Mazeh on 5/25/17.
//  Copyright © 2017 Ali Mazeh. All rights reserved.
//

#include <iostream>
#include <vector>
#include <queue>
#include <set>
#include <chrono> //https://stackoverflow.com/questions/22387586/measuring-execution-time-of-a-function-in-c

#include "State.h"

using namespace std;
using namespace std::chrono;


#define PRINT_STEPS 0 // show every step of the search? Reasonable for size 1
#define PRINT_SOLUTION 1 // show the optimal solution
#define STEP_LIMIT 2000000 // limit the iterations


/*
 Condenses the state into an int.
 Low 32 bits encode the state, high 32 bits contain the gap
 
 Pre: &state must be a valid address of a State object
 Post: returns a hash for the current state in the form of a 64-bit unsigned int
 */

uint64_t stateToHash(const State& state)
{
    uint64_t hash = 0;
    for (int i = 0; i < state.length; ++i)
    {
        if (i == state.gap)
        {
            continue; // a 0 goes to gap
        }
        hash |= state.state[i] << i;
    }
    hash |= (uint64_t)(state.gap) << 32;
    return hash;
}

/*
 Runs the algorithm.
 
 Pre: size number of T or F. The whole puzzle size is then 2* size + 1. Size must be >0
      useUnderestimate must be true A* , useGuess must be false B&B
      uniqueState must be true for A*, uniqueState must be false for B&B
 */
void runAlg(int size, bool useUnderestimate, bool uniqueState = true)
{
    bool done;
    int searchSteps;
    
    State start(size, useUnderestimate);
    
    cout << "Doing " << (useUnderestimate ? "A*" : "B&B") << " with size " << size << ", Starting state: " << endl;
    start.print();
    
    priority_queue<State, vector<State>> queue;
    queue.push(start);
    
    set<uint64_t> bannedStates; // alternative structure for fast exclusion of states already visited or in the queue
    if (uniqueState)
    {
        bannedStates.insert(stateToHash(start));
    }
    
    done = false;
    searchSteps = 0; // safety limit for the while
    
    while (!done && searchSteps < STEP_LIMIT)
    {
        searchSteps++;
        State state = queue.top(); // pull state from the queue
        queue.pop(); // remove the taken element
        
#if PRINT_STEPS
        cout << endl << "-------- step: " << searchSteps << ", distance: " << state.steps << endl;
        state.print();
#endif
        
        // expand the state
        for (Move move : {Move::left, Move::right, Move::jumpLeft, Move::jumpRight})
        {
            if (state.moveLegal(move))
            {
#if PRINT_STEPS
                cout << "Move: ";
                switch (move)
                {
                    case Move::left:
                        cout << "move left";
                        break;
                    case Move::right:
                        cout << "move right";
                        break;
                    case Move::jumpLeft:
                        cout << "jump left";
                        break;
                    case Move::jumpRight:
                        cout << "jump right";
                        break;
                }
                cout << endl;
#endif
                State newState(state, move); // create new state by applying the move
#if PRINT_STEPS
                newState.print();
#endif
                if (newState.isFinal())
                {
                    cout << "The shortest solution has been found.  It took " << searchSteps <<
                    " iterations! Solution is " << newState.steps << " steps long." << endl;
#if PRINT_SOLUTION
                    cout << "\nSolution:" << endl;
                    newState.printPath();
#endif
                    cout << endl;
                    done = true;
                    break;
                }
                // check if the state was visited / is in queue
                if (uniqueState)
                {
                    if (bannedStates.find(stateToHash(newState)) != bannedStates.end())
                    {
                        continue; // the new state is banned, so we skip it.
                    }
                    bannedStates.insert(stateToHash(newState));
                }
                queue.push(newState);
            }
        }
    }
    if (searchSteps == STEP_LIMIT)
    {
        cout << "Reached limit of " << STEP_LIMIT << " iterations before reaching the solution." << endl;
    }
}


int main(void)
{
    high_resolution_clock::time_point t1 , t2;
    int size = 1;
    cout << "Enter problem size (max 15): ";
    cin >> size;
    
    if (size > 15)
    {
        cout << "Game size is too large!" << endl;
        return 0;
    }
    cout << endl;
    
    // initialize time before calling method
//    t1 = high_resolution_clock::now();
//    runAlg(size, false, false); // Branch and Bound
//    // initialize time after method ends
//    t2 = high_resolution_clock::now();
    
    // figure out time passed before and after method call
    auto duration = duration_cast<microseconds>( t2 - t1 ).count();
    
    cout << "Branch and Bound took " << duration << " microseconds to execute.\n\n" << endl;
    
    t1 = high_resolution_clock::now();
    runAlg(size, true, true); // A*
    t2 = high_resolution_clock::now();
    
    duration = duration_cast<microseconds>( t2 - t1 ).count();
    
    cout << "A* algorithm took " << duration << " microseconds to execute." << endl;
    
    cin >> size;
    
    return 0;
}
