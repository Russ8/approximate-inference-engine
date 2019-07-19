#include <iostream>
#include <fstream>
#include <cstdlib>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

//vectors to store samples
vector<vector<bool> > likelihood_samples;
vector<vector<bool> > gibbs_samples;
vector<float> likelihood_samples_weights;
int currentGibbsSample = 0;

//settings
const int num_likelihood_samples = 1000;
const int num_gibbs_samples = 1000;

//given probabilities
const float prob_R0_true = 0.2;
const float prob_Rt_true_given_Rtm1_true = 0.7;
const float prob_Rt_true_given_Rtm1_false = 0.3;
const float prob_Ut_true_given_Rt_true = 0.9;
const float prob_Ut_true_given_Rt_false = 0.2;

//helper function that returns the next word of an input string 
string nextw(string * input, int * i) { 
	char temp;
	temp = (*input)[*i];
	string line = "";

	//this will continue until a word is found or the string is fully parsed
	while(*i<(*input).length()) {
		if((temp == '\n') || (temp == ' ') || (temp == '	')) {
			(*i)++;

			return line;
			

		} else {
			line += temp;
		}
		(*i)++;
		temp = (*input)[*i];
	}
	// nextw() should be finished before this point
	return "err";
}

//help function that returns a random boolean based on the input
float random(float prob) {
	float randomNum = float(rand()%1000)/1000;

	if(randomNum > prob) {
		return false;
	} else {
		return true;
	}
}

//function that creates a likelihood weighted sample
void likelihood_sample(vector<bool> evidence) {

	
	bool previous; //to store probability of Rt-1
	float w = 1.0; //weight initialised to 1
	vector<bool> sample; 
	
	//loop over all evidence variables
	for(int i=0; i<evidence.size(); i++) {

		//sample P (Rt) given the state of Rt-1 and P (Rt | Rt-1).  If this is the first iteraiton, sample from P (R0)
		bool P;
		bool nP;
  		if(i==0) {
			P = random(prob_R0_true);
		} else if(previous == true) {
			P = random(prob_Rt_true_given_Rtm1_true);
		} else {
			P = random(prob_Rt_true_given_Rtm1_false);
		} 
		sample.push_back(P);
		previous = P;

		//the weight is multiplied based upon P (Ut | Rt) 
		if((P==true) && (evidence[i]==true)) {
			w *= prob_Ut_true_given_Rt_true;
		} else if((P==true) && (evidence[i]==false)) {
			w *= 1.0 - prob_Ut_true_given_Rt_true;
		} else if((P==false) && (evidence[i]==true)) {
			w *=prob_Ut_true_given_Rt_false;
		} else {
			w *= 1.0 - prob_Ut_true_given_Rt_false;
		}
		sample.push_back(evidence[i]);
		
	}
	//save the sample
	likelihood_samples.push_back(sample);
	likelihood_samples_weights.push_back(w);
}

//function that prints P (RT| U0 - UT) based on the samples
void likelihood_result(int variable) {
	float ptrue = 0.0;
	float pfalse = 0.0;

	for(int i=0; i<likelihood_samples.size(); i++) {
		for(int j =0; j<likelihood_samples[i].size(); j++) {

		}
	}

	//for each unique sample count the number of identical samples and add to ptrue the count mutiplied by the corresponding weight
	for(int i=0; i<likelihood_samples.size(); i++) { //for all samples
		if(likelihood_samples[i][likelihood_samples[i].size()-2] == true) { //if Rt is true

			int num = 1; //number of identical samples to this one
			for(int j=0; j<likelihood_samples.size(); j++) 
			{ 
					 															
				if(i!=j) 
				{

					for(int k=0; k<likelihood_samples[j].size(); k++) 
					{ //for sample j

						if(likelihood_samples[j][k] != likelihood_samples[i][k]) 
						{ // loop over each sample to test if they are identical

							break;
						} 
						else if(k==likelihood_samples[j].size()-2) 
						{ // the samples are identical 

							num++;
							//erase the samples from the list to avoid duplicate counting
							likelihood_samples.erase(likelihood_samples.begin() + j);

							likelihood_samples_weights.erase(likelihood_samples_weights.begin() + j);

						}
					}
				}
				
			}
			ptrue += num * likelihood_samples_weights[i];
			//increment to ptrue the weight of the samples multiplied by their number
		}  else if(likelihood_samples[i][likelihood_samples[i].size()-2] == false) 
		{ //repeat for if the chosen variable is false

			int num = 1; //number of identical samples to this one

			for(int j=0; j<likelihood_samples.size(); j++) 
			{ //for all samples
				
				if(i!=j) 
				{
					for(int k=0; k<likelihood_samples[j].size(); k++) 
					{ //for sample j
						if(likelihood_samples[j][k] != likelihood_samples[i][k]) 
						{// loop over each sample to test if they are identical
							break;
						} else if(k==likelihood_samples[j].size()-1) 
						{// the samples are identical
							num++;
							likelihood_samples.erase(likelihood_samples.begin() + j);
							likelihood_samples_weights.erase(likelihood_samples_weights.begin() + j);
						}
					}
				}
				
			}
			pfalse += num * likelihood_samples_weights[i];
		}
	}

	//normalise and print the results
	float alpha = 1 / (ptrue + pfalse);
	cout << "likelihood weighted " << ptrue * alpha << " " << pfalse * alpha <<endl;

}

//function that initialises a gibbs sample and returns it
vector<bool> gibbs_init(vector<bool> evidence) {
	//the sample is stored as a boolean vector
	vector<bool> temp;
	for(int i=0; i<evidence.size(); i++) {
		//initialise R0 - Rt randomly to true or false with 0.5 probability
		temp.push_back(random(0.5));
		//retrieve the evidence value
		temp.push_back(evidence[i]);
	}
	return temp;
}


//recursive function that calculates a gibbs sample and stores it
void gibbs_next_state(vector<bool> currentState, int pos) {


	float PRt_given_pRtm1 = 1.0; //variable given parent
	float nPRt_given_pRtm1 = 1.0; //
	float PRtp1_given_pRt = 1.0; //child (Rt+1) given parent
	float nPRtp1_given_pRt = 1.0; //
	float pUt_given_pRt = 1.0; //child (Ut) given parent
	float npUt_given_pRt = 1.0; //

	//find p(Rt|Rt-1)
	//if this is R0 (edge case) then use P(R0) for PRt_given_pRtm1
	if(pos < 2) {
		
		PRt_given_pRtm1 = prob_R0_true;
		nPRt_given_pRtm1 = 1.0f - prob_R0_true;
		
	} else {
		//else get the variable given parent probabilites using prob_Rt_true_given_Rtm1_true/false
		if((currentState[pos-2]==true)) {
			PRt_given_pRtm1 = prob_Rt_true_given_Rtm1_true; //P(Rt=true|Rt-1=true) = 0.7
			nPRt_given_pRtm1 = 1.0 - prob_Rt_true_given_Rtm1_true; //P(Rt=false|Rt-1=true) = 0.3
		} else if( (currentState[pos-2]==false)) {
			PRt_given_pRtm1 = prob_Rt_true_given_Rtm1_false;//P(Rt=true|Rt-1=false) = 0.3
			nPRt_given_pRtm1 = 1.0f - prob_Rt_true_given_Rtm1_false; //P(Rt=true|Rt-1=false) = 0.7
		} 
	}

	//get the evidence variable given parent probabilites using prob_Ut_true_given_Rt_true/false
	if( (currentState[pos+1]==true)) {
		pUt_given_pRt = prob_Ut_true_given_Rt_true;
		npUt_given_pRt = prob_Ut_true_given_Rt_false;
	} else if((currentState[pos+1]==false)) {
		pUt_given_pRt = 1.0f - prob_Ut_true_given_Rt_true;
		npUt_given_pRt = 1.0f - prob_Ut_true_given_Rt_false;
	} 

	//get P(Rt+1|Rt)   = {0.7, 0.3}
	//get the child (Rt+1) variable given parent probabilites using prob_Rt_true_given_Rtm1_true/false
	if(pos < (currentState.size()-2) )
	{
		if((currentState[pos+2]==true)) {
			PRt_given_pRtm1 = prob_Rt_true_given_Rtm1_true;
			nPRt_given_pRtm1 = prob_Rt_true_given_Rtm1_false;
		} else if( (currentState[pos+2]==false)) {
			PRt_given_pRtm1 = 1.0 - prob_Rt_true_given_Rtm1_true;
			nPRt_given_pRtm1 = 1.0f - prob_Rt_true_given_Rtm1_false;
		} 
	}
	//multiply togother the two sets of three probabilities
	float sample_true;
	float sample_false;
	if(PRtp1_given_pRt!=1.0) 
	{
	 	sample_false = nPRt_given_pRtm1 * nPRtp1_given_pRt * npUt_given_pRt;
		sample_true = PRt_given_pRtm1 * PRtp1_given_pRt * pUt_given_pRt;
	}
	else 
	{
		//this is an edge case where there is only one parent
		sample_false = nPRt_given_pRtm1 * npUt_given_pRt;
		sample_true = PRt_given_pRtm1 * pUt_given_pRt;
	}

	//normalise
	float alpha = 1 / (sample_true + sample_false);
	sample_true *= alpha;

	currentState[pos] = random(sample_true);

	pos += 2;
	
	if(pos<currentState.size()) {
		//recursively call the function for the next position
		gibbs_next_state(currentState, pos);
	} else {
		//record the result
		gibbs_samples.at(currentGibbsSample)=currentState;
	}
}

//function that calls the main recursive function above as many times as required
void gen_gibbs_samples(vector<bool> evidence) {
	//repeat for num_gibbs_samples
	for(int i=0; i<num_gibbs_samples; i++) {
		//go over the set of evidence variables for the same sample multiple times
		for(int j=0; j<10; j++) {
			if(j==0) {
				//set up initial state
				vector<bool> temp;
				gibbs_samples.push_back(temp);
				gibbs_next_state(gibbs_init(evidence), 0);
			} else {
				gibbs_next_state(gibbs_samples.at(currentGibbsSample), 0);
			}
		}
		currentGibbsSample ++ ;
	}
	
}

//function that prints the gibbs result
void gibbs_result(int variable) {

	//occurance of Rt = true/false
	int num_true=0;
	int num_false=0;

	//count the number of times Rt is true and false, 
	for(int i=0; i<gibbs_samples.size(); i++) {
		if(gibbs_samples[i][gibbs_samples[0].size()-2] == true) {
			num_true ++;
		} else {
			num_false ++;
		}
	}
	
	//normalise
	float alpha = 1 / (float(num_true) + float(num_false));
	//print the results
	cout << "gibbs " << float(num_true) * alpha << " " << float(num_false) * alpha << endl;



}


int main(int argc, char **argv) {
	//set a random seed of rand()
	srand (time(NULL));

	//check input parameters
	if(argc<2) {
		cout << "incorrect parameters" << endl;
		return 0;
	} 

	//taking in input string and reading the file at that location
	string input;
	ifstream inputFile;
	inputFile.open(argv[1]);
	stringstream inputStream;
	inputStream << inputFile.rdbuf();
	inputFile.close();
	input = inputStream.str();

	vector<bool> evidence;
	
	//setup variables for nextw() function
	int i=0; 
	string * in = &input;

	//parse the input string
	string temp;
	while((temp = nextw(in, &i)) != "" ) {
		evidence.push_back(atoi(temp.c_str()) == 1);
	}

	//call the functions that generate the samples
	for(int j=0; j<num_likelihood_samples; j++) {
		likelihood_sample(evidence);
	}

	likelihood_result(evidence.size()-1);

	//call the functions that print the result
	gen_gibbs_samples(evidence);

	gibbs_result(evidence.size()-2);


}










