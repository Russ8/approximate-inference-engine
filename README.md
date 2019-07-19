# approximate-inference-engine  
 An approximate inference engine for the Hidden Markov Model.  

A Markov model is a stochastic model used to model randomly changing systems. It is assumed that future states depend only on the current state, not on the events that occurred before it. A Hidden Markov Model (HMM) is a statistical Markov model in which the system being modeled is assumed to be a Markov process with unobservable (i.e. hidden) states.  

## Instructions  
Build:  
```$ make```  
Which creates an executable, "inference"   

Run the program:   
```$ ./inference prev_states.txt```   
Where prev_states.txt contains a binary sequence representing the results of all previous states.  
The program outputs the probability that the next state after the given states will yield true.  
