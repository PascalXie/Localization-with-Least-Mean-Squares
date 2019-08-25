#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <random>
#include <ctime>

#include "OptimizationLeastMeanSquares.hh"

using namespace std;

vector<int> AnchorIDs;
vector<int> NodeIDs;
vector<double> S_anchors, S_nodes;
vector<double> axs, ays, xxs, xys;

int NumberAnchors = 0;
int NumberNodes = 0;

double sigma_TiAj_dBm = 0;
double Threshold_dBm = 0;

double Power_node;
double Power_ref;
double PathLossExponent;
double d0;

bool PowerRSSI_singleNode(int NodeID,vector<double> &results);

int main()
{
	cout<<"Hello "<<endl;

	//
	string filename0 = "../Data_SigmaAndThreshold.txt";
	ifstream file0(filename0.c_str());

	if(file0.fail())
	{
		cout<<"Can not find the file0 \" "<<filename0<<" \""<<endl;
		return 0;
	}

	string temp0;

	file0>>temp0>>sigma_TiAj_dBm;
	cout<<temp0<<" "<<sigma_TiAj_dBm<<endl;
	file0>>temp0>>Threshold_dBm;
	cout<<temp0<<" "<<Threshold_dBm<<endl;

	file0.close();



	//
	string filename1 = "../Data_PathLossExponent.txt";
	ifstream file1(filename1.c_str());

	if(file1.fail())
	{
		cout<<"Can not find the file1 \" "<<filename1<<" \""<<endl;
		return 0;
	}

	string temp1;

	file1>>temp1>>Power_node;
	file1>>temp1>>Power_ref;
	file1>>temp1>>PathLossExponent;
	file1>>temp1>>d0;

	cout<<"Power_node "<<Power_node<<endl;
	cout<<"Power_ref "<<Power_ref<<endl;
	cout<<"PathLossExponent "<<PathLossExponent<<endl;
	cout<<"d0 "<<d0<<endl;


	//
	string filename = "../observations.txt";
	ifstream file(filename.c_str());

	if(file.fail())
	{
		cout<<"Can not find the file \" "<<filename<<" \""<<endl;
		return 0;
	}

	string temp;
	file>>temp>>NumberAnchors>>temp>>NumberNodes;
	cout<<"NumberAnchors "<<NumberAnchors<<" ; NumberNodes "<<NumberNodes<<endl;

	int AnchorID = 0;
	int NodeID = 0;
	double ax = 0; 
	double ay = 0; 
	double az = 0;
	double xx = 0;
	double xy = 0;
	double xz = 0;
	double S_anchorOb = 0; // signal Power
	double S_node = 0; // signal power

	// random for signal noise
	default_random_engine engine_(time(0));
	double mean = 0; // Expectation
	double sigma = sigma_TiAj_dBm; // standard deviation, dBm, {4, 8 ,12}
	normal_distribution<double> normal(mean, sigma);
	cout<<"random for signal noise : Sigma "<<sigma<<endl;

	while(!file.eof())
	{
		file>>AnchorID>>ax>>ay>>az>>S_anchorOb>>NodeID>>xx>>xy>>xz>>S_node;

		if(file.eof()) break;

		//cout<<"AnchorID "<<AnchorID<<", ax "<<ax<<", ay "<<ay<<", az "<<az<<"; Power "<<S_anchorOb<<endl;
		//cout<<"NodeID "<<NodeID<<", xx "<<xx<<", xy "<<xy<<", xz "<<xz<<"; Power "<<S_node<<endl;

		// S_anchorOb randomized 
		S_anchorOb += normal(engine_);

		AnchorIDs.push_back(AnchorID);
		NodeIDs.push_back(NodeID);
		axs.push_back(ax);
		ays.push_back(ay);
		xxs.push_back(xx);
		xys.push_back(xy);
		S_anchors.push_back(S_anchorOb);
		S_nodes.push_back(S_node);
	}

	file.close();

	// localization
	ofstream file3("log_OptimizationResults.txt");
	cout<<"localization"<<endl;
	for(int j=0;j<NumberNodes;j++)
	{
		int NodeID = j;
		vector<double> results;
		PowerRSSI_singleNode(NodeID,results);

		//cout<<"sigma_TiAj_dBm "<<sigma_TiAj_dBm<<endl;
		//cout<<"Threshold_dBm "<<Threshold_dBm<<endl;
		//cout<<"NodeID "<<NodeID<<", Node Position : "<<results[0]<<", "<<results[1]<<endl;
		file3<<NodeID<<" "<<results[0]<<" "<<results[1]<<endl;
	}
	file3.close();




	return 1;
}


//
bool PowerRSSI_singleNode(int NodeID,vector<double> &results)
{
	// Optimization
	OptimizationLeastMeanSquares * lms= new OptimizationLeastMeanSquares("LMS");

	// get observations
	for(int i=0;i<NumberAnchors;i++)
	{
		int AnchorID = i;
		int S_anchorObID = AnchorID*NumberNodes + NodeID;
		double S_ao = S_anchors[S_anchorObID];
		if(S_ao<Threshold_dBm) continue;

		// debug
		double dx = axs[S_anchorObID] - xxs[NodeID];
		double dy = ays[S_anchorObID] - xys[NodeID];
		double d_true = sqrt(dx*dx + dy*dy);

		vector<double> anchor;
		double ax = axs[S_anchorObID];
		double ay = ays[S_anchorObID];
		anchor.push_back(ax);
		anchor.push_back(ay);

		// compute distance
		double part1 = Power_ref-S_ao;
		double part2 = 10.*PathLossExponent;
		double exponent = part1/part2;
		double d = d0 * pow(10.,exponent);

		lms->AddObservation(anchor, d);

		/*
		// deubg
		cout<<"NodeID "<<NodeID<<", AnchorID "<<AnchorID<<", distance "<<d<<", d_true "<<d_true<<endl;
		*/

	}

	lms->Initiate();

	lms->GetOptimizationResult(results);
	cout<<"True value: "<<xxs[NodeID]<<", "<<xys[NodeID]<<endl;
	cout<<"Result Node: "<<results[0]<<", "<<results[1]<<endl;


	return true;
}
