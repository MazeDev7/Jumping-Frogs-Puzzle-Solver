//
//  State.h
//
//  Created by Ali Mazeh on 5/25/17.
//  Copyright © 2017 Ali Mazeh. All rights reserved.
//

#pragma once

using namespace std;

// possible moves in the game
enum class Move
{
    left, // move thing right of the gap (higher index) to the left
    right, // move thing left of the gap (higher index) to the right
    jumpLeft, // etc..
    jumpRight,
    none
};


// parent class containing all stuff common to a game state in both algorithms.
 
class State
{
public:
    bool* state; // 0 for F, 1 for T
    int length; // total size, not the toad/frog number
    int gap; // position of the gap
    int steps; // steps done so far
    bool useUnderestimate;
    int underestimate;
    vector<Move> path;
    
    State() : length(0), state(nullptr), gap(0), steps(0), useUnderestimate(false), underestimate(0) {}; // default constructor, not used
    
    // copy constructor, used when pulling from the queue (= queue.top())
    State(const State& other) :
    length(other.length), steps(other.steps), gap(other.gap), useUnderestimate(other.useUnderestimate), underestimate(other.underestimate), path(other.path)
    {
        // first copy the state
        this->state = new bool[length];
        for (int i = 0; i < this->length; ++i)
        {
            this->state[i] = other.state[i];
        }
    }
    
    
    // constructor of a starting state
    State(int size, bool useGuess) : length(size * 2 + 1), gap(size), steps(0), useUnderestimate(useGuess), underestimate(0)
    {
        // init the starting position
        this->state = new bool[length]; // size F and T and the gap
        for (int i = 0; i < size; ++i)
        {
            this->state[i] = 0;
        }
        for (int i = size + 1; i < 2 * size + 1; ++i)
        {
            this->state[i] = 1;
        }
    }
    
    /* constructor of a state evolved from another state by making the given move.
     If the move is not possible, creates an inconsistent state and reports an error to cout.
     */
    State(const State& other, Move move) :
    length(other.length), steps(other.steps + 1), gap(other.gap), useUnderestimate(other.useUnderestimate), underestimate(other.underestimate), path(other.path)
    {
        if (!other.moveLegal(move))
        {
            cout << "Error - illegal move" << endl;
            return;
        }
        
        // copy the state
        this->state = new bool[length];
        
        for (int i = 0; i < this->length; ++i)
        {
            this->state[i] = other.state[i];
        }
        // perform the move
        switch (move)
        {
            case Move::left:
                // move item right of this->gap to the this->gap position and move the this->gap right
                this->state[this->gap] = this->state[this->gap + 1];
                ++this->gap;
                break;
            case Move::right:
                this->state[this->gap] = this->state[this->gap - 1];
                --this->gap;
                break;
            case Move::jumpLeft:
                this->state[this->gap] = this->state[this->gap + 2];
                this->gap += 2;
                break;
            case Move::jumpRight:
                this->state[this->gap] = this->state[this->gap - 2];
                this->gap -= 2;
                break;
        }
        
        path.push_back(move);
        
        if (useUnderestimate)
        { // For A*, otherwise guess stays at 0
            updateUnderestimate();
        }
    }
    
    /* Checks if the current move is possible
     
     Pre: move must be a valid "move"
     Post: returns false if move is not possible. returns true otherwise
     */
    bool moveLegal(Move move) const
    {
        switch (move)
        {
            case Move::left:
                if (this->gap == this->length - 1) // gap is last, can't move from the right
                {
                    return false;
                }
                break;
            case Move::right:
                if (this->gap == 0) // this->gap is first
                {
                    return false;
                }
                break;
            case Move::jumpLeft:
                // must have two items right of the this->gap and different thing between
                if (this->gap >= this->length - 2 || this->state[this->gap + 1] == this->state[this->gap + 2])
                {
                    return false;
                }
                break;
            case Move::jumpRight:
                // must have two items left of the this->gap and different thing between
                if (this->gap <= 1 || this->state[this->gap - 1] == this->state[this->gap - 2])
                {
                    return false;
                }
                break;
            default:
                return false;
        }
        return true;
    }
    
    /* Checks if  the game is finished in the current state
     
     Pre: No input parameter
     Post: returns true if the final solution is found. returns false otherwise
     */
    bool isFinal(void)
    {
        bool finished = true;
        int size = this->length / 2;
        
        // loop to check if left side still contains frogs
        for (int i = 0; i < size; ++i)
        {
            if (this->state[i] == 0 || this->gap == i)
            {
                finished = false;
            }
        }
        // loop to check if left side still contains frogs
        for (int i = size + 1; i < this->length; ++i)
        {
            if (this->state[i] == 1 || this->gap == i)
            {
                finished = false;
            }
        }
        return finished;
    }
    
    /* recalculate guess of distance to the end
     
     Pre: No input parameter
     Post: modifies the current underestimate
     */
    void updateUnderestimate(void)
    {
        this->underestimate = 0;
        int size = this->length / 2;
        
        // count number of frogs out of position
        for (int i = 0; i < size; ++i)
        {
            if (this->state[i] == 0 || gap == i)
            {
                this->underestimate++;
            }
        }
        // count number of toads out of position
        for (int i = size + 1; i < this->length; ++i)
        {
            if (this->state[i] == 1 || gap == i)
            {
                this->underestimate++;
            }
        }
    }
    
    /* This method returns the score of the state
     
     Pre: No input parameter
     Post: returns the current steps + underestimate
     */
    int score(void) const { return steps + underestimate; } // score of the state. Different implementation by used algorithm.
    
    /* Operator overloading > symbol to allow comparison of objects for priority queue sorting
     
     Pre: other must contain a valid address of a State object
     Post: returns true if left objects score is less then right objects score
     
     */
    bool operator<(const State& other) const
    {
        // lower score is better -> higher in order.
        return this->score() > other.score();
    }
    
    /* operator overloading = to allow deep copy of object
     
     Pre: other must contain a valid address of a State object
     Post: returns the the "left object" which contains data of the "right object"
     */
    State& operator=(const State& rhs) // for the = when pulled out of queue, must be implemented explicitely
    {
        if (this != &rhs)
        {
            this->steps = rhs.steps;
            this->length = rhs.length;
            this->gap = rhs.gap;
            this->useUnderestimate = rhs.useUnderestimate;
            this->underestimate = rhs.underestimate;
            delete[] this->state;
            this->state = new bool[length];
            
            for (int i = 0; i < this->length; ++i)
            {
                this->state[i] = rhs.state[i];
            }
            this->path = rhs.path;
        }
        return *this;
    }
    
    /* Prints out the current state
     
     Pre:  No input, but must have an active State object
     Post: Prints out the state as e.g. 0 _ 1 0 1
     */
    void print(void) const
    {
        for (int i = 0; i < this->length; ++i)
        {
            if (this->gap == i)
            {
                 cout << "_ ";
            }
            else
            {
                 cout << this->state[i] << " ";
            }
        }
         cout <<  endl;
    }
    
    /* prints the move set leading to this state

     Pre:  No input, but must have an active State object
     Post: prints the move set leading to this state
     */
    void printPath()
    {
        for (Move move : path)
        {
            switch (move)
            {
                case Move::left:  cout << "move left" <<  endl; break;
                case Move::right:  cout << "move right" <<  endl; break;
                case Move::jumpLeft:  cout << "jump left" <<  endl; break;
                case Move::jumpRight:  cout << "jump right" <<  endl; break;
            }
        }
    }
    // Destructor
    ~State() { delete[] state; };
};
