/*
* This is the main program of our trading system
* Author: Tengxiao Fan
*/
#include "functionalities.hpp"
#include <iostream>
#include <iomanip>
#include "soa.hpp"
#include "positionservice.hpp"
#include "pricingservice.hpp"
#include "marketdataservice.hpp"
#include "tradebookingservice.hpp"
#include "riskservice.hpp"
#include "algoexecutionservice.hpp"
#include "algostreamingservice.hpp"
#include "executionservice.hpp"
#include "streamingservice.hpp"
#include "GUIService.hpp"
#include "inquiryservice.hpp"
#include "historicaldataservice.hpp"
#include "DataGeneration.hpp"


int main()
{
	//Data Generation
	std::cout << "Generation Start" << endl;
	GeneratePriceData();
	GenerateTradeData();
	GenerateMarketData();
	GenerateInquiries();
	std::cout << "Generation End" << endl;

	//Make services
	PricingService<Bond> pricingservice;
	TradeBookingService<Bond> tradebookingservice;
	PositionService<Bond> positionservice;
	RiskService<Bond> riskservice;
	MarketDataService<Bond> marketdataservice;
	AlgoExecutionService<Bond> algoexecutionservice;
	AlgoStreamingService<Bond> algostreamingservice;
	ExecutionService<Bond> executionservice;
	StreamingService<Bond> streamingservice;
	GUIService<Bond> guiservice;
	InquiryService<Bond> inquiryservice;
	HistoricalDataService<Position<Bond>> historicalpositionservice("POSITION");
	HistoricalDataService<PV01<Bond>> historicalriskservice("RISK");
	HistoricalDataService<ExecutionOrder<Bond>> historicalexecutionservice("EXECUTION");
	HistoricalDataService<PriceStream<Bond>> historicalstreamservice("STREAMING");
	HistoricalDataService<Inquiry<Bond>> historicalinquiryservice("INQUIRY");
	

	//Add the listeners
	tradebookingservice.AddListener(positionservice.GetTradeBookingListener());
	positionservice.AddListener(riskservice.GetPositionListener());
	marketdataservice.AddListener(algoexecutionservice.GetMarketDataListener());
	pricingservice.AddListener(algostreamingservice.GetPricingListener());
	algoexecutionservice.AddListener(executionservice.GetAlgoExecutionListener());
	executionservice.AddListener(tradebookingservice.GetExecutionListener());
	algostreamingservice.AddListener(streamingservice.GetAlgoStreamingListener());
	pricingservice.AddListener(guiservice.GetPricingListener());
	positionservice.AddListener(historicalpositionservice.GetDataListener());
	riskservice.AddListener(historicalriskservice.GetDataListener());
	executionservice.AddListener(historicalexecutionservice.GetDataListener());
	streamingservice.AddListener(historicalstreamservice.GetDataListener());
	inquiryservice.AddListener(historicalinquiryservice.GetDataListener());
	

	//Import data
	ifstream tradeData("trades.txt");
	ifstream priceData("prices.txt");
	ifstream marketData("marketdata.txt");
	ifstream inquiryData("inquiries.txt");
	tradebookingservice.GetConnector()->Subscribe(tradeData);
	pricingservice.GetConnector()->Subscribe(priceData);
	marketdataservice.GetConnector()->Subscribe(marketData);
	inquiryservice.GetConnector()->Subscribe(inquiryData);

	//std::cout << marketdataservice.GetData("TMUBMUSD02Y").GetOfferStack()[2].GetPrice() << std::endl;
	//std::cout << "end" << std::endl;
	//Test data
	//std::cout<<riskservice.GetData("TMUBMUSD02Y").GetPV01() << endl;
	cout << "Everything Done" << endl;
	
}