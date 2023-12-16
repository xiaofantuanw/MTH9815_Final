/*
* Functionalities.hpp
* This file includes all the functions that will be used
* Author: Tengxiao Fan
*/

#ifndef Functionalities_HPP
#define Functionalities_HPP
#include <string>
#include<random>
#include "soa.hpp"
#include "products.hpp"


//This turns a fractional price into a double price
double FractionaltoPrice(string s)
{
	string part1="";
	string part2="";
	int partint = 1;

	for (auto i = s.begin(); i != s.end(); i++)
	{
		if ((*i) == '-')
		{
			partint = 0;
			continue;
		}
		if (partint == 1)
		{
			part1.push_back(*i);
		}
		else
		{
			part2.push_back(*i);
		}
	}
	string part2_32 = "";
	string part2_256 = "";
	if (part2[2] == '+')
		part2[2] = '4';
	part2_32.push_back(part2[0]);
	part2_32.push_back(part2[1]);
	part2_256.push_back(part2[2]);
	double Priceint = stod(part1);
	double Pricedec = stod(part2_32) / 32.0 + stod(part2_256) / 256.0;
	return Priceint + Pricedec;
}


//This function turns a double price to a fractional price
string PricetoFraction(double price)
{
	int price1 = floor(price);
	int price2 = floor((price - price1) * 256);
	int price3 = floor(price2 / 8.0);
	int price4 = price2 % 8;

	string str1 = to_string(price1);
	string str2 = to_string(price3);
	string str3 = to_string(price4);
	if (price3 < 10) str2 = "0" + str2;
	if (price4 == 4) str3 = "+";
	string result = str1 + "-" + str2 + str3;
	return result;
}


// Make the bonds of different matures
Bond MakeBond(string cusip)
{
	//std::cout << "wow" << std::endl;
	Bond bond;
	
	if (cusip == "TMUBMUSD02Y") {
		//std::cout << "wow" << std::endl;
		bond = Bond(cusip, CUSIP, "T", 0.04875, from_string("2025/12/31"));
	}
	else if (cusip == "TMUBMUSD03Y") 
		bond = Bond(cusip, CUSIP, "T", 0.04625, from_string("2026/12/31"));
	else if (cusip == "TMUBMUSD05Y")
		bond = Bond(cusip, CUSIP, "T", 0.04375, from_string("2028/12/31"));
	else if (cusip == "TMUBMUSD07Y") 
		bond = Bond(cusip, CUSIP, "T", 0.04375, from_string("2030/12/31"));
	else if (cusip == "TMUBMUSD10Y") 
		bond = Bond(cusip, CUSIP, "T", 0.04500, from_string("2033/12/31"));
	else if (cusip == "TMUBMUSD20Y") 
		bond = Bond(cusip, CUSIP, "T", 0.04750, from_string("2043/12/31"));
	return bond;
}

/*
* Calculate the PV01 of the bonds
*/
double CaluculatePV01(string cusip)
{
	double pv01=0;
	if (cusip == "TMUBMUSD02Y")
		pv01 = 0.02;
	else if (cusip == "TMUBMUSD03Y")
		pv01 = 0.03;
	else if (cusip == "TMUBMUSD05Y")
		pv01 = 0.05;
	else if (cusip == "TMUBMUSD07Y")
		pv01 = 0.07;
	else if (cusip == "TMUBMUSD10Y")
		pv01 = 0.1;
	else if (cusip == "TMUBMUSD20Y")
		pv01 = 0.2;
	return pv01;
}

// Generate random IDs.
string GenerateId()
{
	string base = "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	string id = "";
	for (int i=0;i<9;i++)
	{
		int random_num = rand() % (base.size());
		id.push_back(base[random_num]);
	}
	return id;
}

#endif // !Functionalities_HPP

