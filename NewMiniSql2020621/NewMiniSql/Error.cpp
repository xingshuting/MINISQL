#include "Error.h"
#include <vector>
#include"Error.h"

	void Error::PrintError() const
	{
		// ����쳣
		std::cout << ErrorInfo;
	}


	Error::Error(const std::string s)
	{
		//ErrorInfo = "Command format error!";
		ErrorInfo = s;
	}



void DispatchError(const Error &error)
{
	error.PrintError();
}



