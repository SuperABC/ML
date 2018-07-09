#pragma once
#include<eigen\eigen>

class RNN {
private:
	Eigen::MatrixXd U, V, W;
public:
	RNN();
	~RNN();
};

