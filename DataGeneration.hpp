/*
* DataGeneration.hpp
* Generate the Data we need for the trading system
* Author: Tengxiao Fan
*/
#ifndef DataGeneration_HPP
#define DataGeneration_HPP
#include "soa.hpp"
#include "tradebookingservice.hpp"

/*
* Generate prices.txt
*/
void GeneratePriceData()
{
	ofstream file;
	file.open("prices.txt");
	vector<string> cusips{"TMUBMUSD02Y","TMUBMUSD03Y","TMUBMUSD05Y","TMUBMUSD07Y","TMUBMUSD10Y","TMUBMUSD20Y"};
	//std::cout << cusips[5] << endl;
	for (int i = 0; i < 6; i++)
	{
		double midprice = 99.0,bidprice,offerprice;
		bool direction = 1;
		bool sprd = 1;
		for (int j = 0; j < 1000; j++)
		{
			if (sprd == 1)
			{
				sprd = 0;
				bidprice = midprice - 1. / 256.;
				offerprice = midprice + 1. / 256.;
			}
			else
			{
				sprd = 1;
				bidprice = midprice - 1. / 128.;
				offerprice = midprice + 1. / 128.;
			}
			file << cusips[i] << "," << PricetoFraction(bidprice) << "," << PricetoFraction(offerprice) << endl;
			if (direction == 1)
			{
				midprice += 1. / 256.;
			}
			else
			{
				midprice -= 1. / 256.;
			}
			if (abs(midprice - 99.) < 1e-6 || abs(midprice - 101.) < 1e-6) direction = 1 - direction;
		}
	}

	file.close();
}

/*
* Generate trades.txt
*/
void GenerateTradeData()
{
	int tradecount = 0;
	ofstream file;
	file.open("trades.txt");
	vector<string> cusips{ "TMUBMUSD02Y","TMUBMUSD03Y","TMUBMUSD05Y","TMUBMUSD07Y","TMUBMUSD10Y","TMUBMUSD20Y" };
	for (int i = 0; i < 6; i++)
	{
		for (int j = 0; j < 10; j++)
		{
			string tradeid = GenerateId();
			string side = "SELL";
			string price = PricetoFraction(100.0);
			if (tradecount % 2)
			{
				side = "BUY";
				price = PricetoFraction(99.0);
			}
				
			string book = "TRSY3";
			if (tradecount % 3 == 1) book = "TRSY1";
			else if (tradecount % 3 == 2) book = "TRSY2";
			long quant = ((tradecount % 5) + 1) * 1000000;
			file << cusips[i] << "," << tradeid << "," << price << "," << book << "," << quant << "," << side << endl;
			tradecount++;
		}
	}
	file.close();
}

/*
* Generate marketdata.txt
*/
void GenerateMarketData()
{
	ofstream file;
	file.open("marketdata.txt");
	vector<string> cusips{ "TMUBMUSD02Y","TMUBMUSD03Y","TMUBMUSD05Y","TMUBMUSD07Y","TMUBMUSD10Y","TMUBMUSD20Y" };
	//std::cout << cusips[5] << endl;
	long count = 0;
	for (int i = 0; i < 6; i++)
	{
		double midprice = 99.0, bidprice, offerprice;
		bool direction = 1;
		bool sprd = 1;
		for (int j = 0; j < 10000; j++)
		{
			long quantity = 1000000 * ((count % 5) + 1);
			if (sprd == 1)
			{
				sprd = 0;
				bidprice = midprice - 1. / 256.;
				offerprice = midprice + 1. / 256.;
			}
			else
			{
				sprd = 1;
				bidprice = midprice - 1. / 128.;
				offerprice = midprice + 1. / 128.;
			}
			file << cusips[i] << "," << PricetoFraction(bidprice) << "," << quantity <<",BID"<< endl;
			file << cusips[i] << "," << PricetoFraction(offerprice) << "," << quantity << ",OFFER" << endl;
			if (direction == 1)
			{
				midprice += 1. / 256.;
			}
			else
			{
				midprice -= 1. / 256.;
			}
			if (abs(midprice - 99.) < 1e-6 || abs(midprice - 101.) < 1e-6) direction = 1 - direction;
			count++;
		}
	}

	file.close();
}


/*
* Generate inquiries.txt
*/
void GenerateInquiries()
{
	int tradecount = 0;
	ofstream file;
	file.open("inquiries.txt");
	vector<string> cusips{ "TMUBMUSD02Y","TMUBMUSD03Y","TMUBMUSD05Y","TMUBMUSD07Y","TMUBMUSD10Y","TMUBMUSD20Y" };
	for (int i = 0; i < 6; i++)
	{
		for (int j = 0; j < 10; j++)
		{
			string tradeid = GenerateId();
			string side = "SELL";
			string price = PricetoFraction(100.0);
			if (tradecount % 2)
			{
				side = "BUY";
				price = PricetoFraction(99.0);
			}

			long quant = ((tradecount % 5) + 1) * 1000000;
			file << tradeid << "," << cusips[i] << "," << side  << "," << quant << "," << price <<",RECEIVED"<< endl;
			tradecount++;
		}
	}
	file.close();
}

#endif // !DataGeneration_HPP
