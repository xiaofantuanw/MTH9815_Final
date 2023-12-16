/**
* algoexecutionservice.hpp
* Defines the data types and Service for algo executions.
*
* @author Breman Thuraisingham & Tengxiao Fan
*/

#ifndef ALGO_EXECUTION_SERVICE_HPP
#define ALGO_EXECUTION_SERVICE_HPP


#include <string>
#include "soa.hpp"
#include "marketdataservice.hpp"


template<typename T>
class ExecutionOrder
{
private:
	T product;
	PricingSide side;
	string orderId;
	double price;
	long quantity;
public:
	//Ctor and Dtor
	ExecutionOrder() {}
	ExecutionOrder(const T& p, PricingSide s, string Id, double pr, long q)
	{
		product = p;
		side = s;
		orderId = Id;
		price = pr;
		quantity = q;
	}
	~ExecutionOrder() = default;

	// Get the product
	const T& GetProduct() const
	{
		return product;
	}

	// Get the pricing side
	PricingSide GetPricingSide() const
	{
		return side;
	}

	// Get the order ID
	const string& GetOrderId() const
	{
		return orderId;
	}

	// Get the price on this order
	double GetPrice() const
	{
		return price;
	}

	//Get the quantity
	long GetQuantity() const
	{
		return quantity;
	}

	//Output function
	ostream& Output(ostream& file)
	{
		file << GetProduct().GetProductId() << "," << GetOrderId() << ",";
		if (GetPricingSide() == BID)
		{
			file << "BID,";
		}
		else if (GetPricingSide() == OFFER)
		{
			file << "OFFER,";
		}
		file << GetPrice() << "," << GetQuantity() << endl;
		return file;
	}
};



template<typename T>
class AlgoExecution
{
private:
	ExecutionOrder<T>* executionorder;



public:
	//Ctor and Dtor
	AlgoExecution() {}
	AlgoExecution(const T& p, PricingSide s, string Id, double pr, long q)
	{
		executionorder = new ExecutionOrder<T>(p, s, Id, pr, q);
	}

	//Getter
	ExecutionOrder<T>* GetExecutionOrder() const
	{
		return executionorder;
	}
};

/*
* Pre declaration of listener
*/

template<typename T>
class AlgoExecutionMarketDataListener;

/*
* Service for algo-execution
*/
template<typename T>
class AlgoExecutionService : public Service<string, AlgoExecution<T>>
{
private:
	map<string, AlgoExecution<T>> algoexecutionmap;
	vector<ServiceListener<AlgoExecution<T>>*> listeners;
	AlgoExecutionMarketDataListener<T>* MarketDataListener;
	bool BidOrOffer;

public:
	//Ctor and Dtor
	AlgoExecutionService()
	{
		BidOrOffer = 1;
		algoexecutionmap = map<string, AlgoExecution<T>>();
		listeners = vector<ServiceListener<AlgoExecution<T>>*>();
		MarketDataListener = new AlgoExecutionMarketDataListener<T>(this);
	}
	~AlgoExecutionService() = default;

	// Get data on our service given a key
	AlgoExecution<T>& GetData(string key)
	{
		return algoexecutionmap[key];
	}

	// The callback that a Connector should invoke for any new or updated data
	void OnMessage(AlgoExecution<T>& data)
	{
		string key = data.GetExecutionOrder()->GetProduct().GetProductId();
		algoexecutionmap[key] = data;
		//Call all the listeners
		for (auto i = listeners.begin(); i != listeners.end(); i++)
		{
			(*i)->ProcessAdd(data);
		}
	}

	// Add a listener to the Service for callbacks on add, remove, and update events for data to the Service
	void AddListener(ServiceListener<AlgoExecution<T>>* listener)
	{
		listeners.push_back(listener);
	}

	// Get all listeners on the Service
	const vector<ServiceListener<AlgoExecution<T>>*>& GetListeners() const
	{
		return listeners;
	}

	// Get the Market data listener of the service
	AlgoExecutionMarketDataListener<T>* GetMarketDataListener()
	{
		return MarketDataListener;
	}

	//Execute Order
	void ExecuteOrder(OrderBook<T>& odb)
	{
		T product = odb.GetProduct();
		string productid = product.GetProductId();
		string orderid = GenerateId();
		PricingSide side;
		double price;
		long quantity;

		BidOffer bidoffer = odb.GetBidOffer();
		Order bidorder = bidoffer.GetBidOrder();
		Order offerorder = bidoffer.GetOfferOrder();
		if (offerorder.GetPrice() - bidorder.GetPrice() <= 1.0 / 128.0)
		{
			if (BidOrOffer == 1)
			{
				BidOrOffer = 0;
				price = bidorder.GetPrice();
				quantity = bidorder.GetQuantity();
				side = BID;
			}
			else
			{
				BidOrOffer = 1;
				price = offerorder.GetPrice();
				quantity = offerorder.GetQuantity();
				side = OFFER;
			}
			AlgoExecution<T> algoEx(product, side, orderid, price, quantity);
			//cout << orderid << "," << side << "," << price << "," << quantity << endl;
			OnMessage(algoEx);
		}
	}
};

template<typename T>
class AlgoExecutionMarketDataListener :public ServiceListener<OrderBook<T>>
{
private:
	AlgoExecutionService<T>* service;

public:
	AlgoExecutionMarketDataListener(AlgoExecutionService<T>* s)
	{
		service = s;
	}
	// Listener callback to process an add event to the Service
	virtual void ProcessAdd(OrderBook<T>& data)
	{
		service->ExecuteOrder(data);
	}

	// Listener callback to process a remove event to the Service
	virtual void ProcessRemove(OrderBook<T>& data){}

	// Listener callback to process an update event to the Service
	virtual void ProcessUpdate(OrderBook<T>& data){}

};


#endif // !ALGO_EXECUTION_SERVICE_HPP
