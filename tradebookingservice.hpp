/**
 * tradebookingservice.hpp
 * Defines the data types and Service for trade booking.
 *
 * @author Breman Thuraisingham & Tengxiao Fan
 */
#ifndef TRADE_BOOKING_SERVICE_HPP
#define TRADE_BOOKING_SERVICE_HPP

#include <string>
#include <vector>
#include "soa.hpp"
#include "executionservice.hpp"

// Trade sides
enum Side { BUY, SELL };

/**
 * Trade object with a price, side, and quantity on a particular book.
 * Type T is the product type.
 */
template<typename T>
class Trade
{

public:

  // ctor for a trade
	Trade() {}
  Trade(const T &_product, string _tradeId, double _price, string _book, long _quantity, Side _side);

  // Get the product
  const T& GetProduct() const;

  // Get the trade ID
  const string& GetTradeId() const;

  // Get the mid price
  double GetPrice() const;

  // Get the book
  const string& GetBook() const;

  // Get the quantity
  long GetQuantity() const;

  // Get the side
  Side GetSide() const;

private:
  T product;
  string tradeId;
  double price;
  string book;
  long quantity;
  Side side;

};

/*
* Pre declarations
*/
template<typename T>
class TradeBookingConnector;
template<typename T>
class TradeBookingExecutionListener;


/**
 * Trade Booking Service to book trades to a particular book.
 * Keyed on trade id.
 * Type T is the product type.
 */
template<typename T>
class TradeBookingService : public Service<string,Trade <T> >
{
private:
	map<string, Trade<T>> trades;
	vector<ServiceListener<Trade<T>>*> listeners;
	TradeBookingConnector<T>* connector;
	TradeBookingExecutionListener<T>* execution_listener;

public:
	//Ctor and Dtors
	TradeBookingService()
	{
		trades = map<string, Trade<T>>();
		listeners = vector<ServiceListener<Trade<T>>*>();
		connector = new TradeBookingConnector<T>(this);
		execution_listener = new TradeBookingExecutionListener<T>(this);
	}
	~TradeBookingService() = default;

	// Get data on our service given a key
	virtual Trade<T>& GetData(string key)
	{
		return trades[key];
	}

	// The callback that a Connector should invoke for any new or updated data
	virtual void OnMessage(Trade<T>& data)
	{
		//Update the trade data
		string key = data.GetTradeId();
		trades[key] = data;

		//Notify all the listeners
		for (auto i = listeners.begin(); i != listeners.end(); i++)
		{
			(*i)->ProcessAdd(data);
		}
	}

	// Add a listener to the Service for callbacks on add, remove, and update events
	// for data to the Service.
	virtual void AddListener(ServiceListener<Trade<T>>* listener)
	{
		listeners.push_back(listener);
	}

	// Get all listeners on the Service.
	virtual const vector< ServiceListener<Trade<T>>* >& GetListeners() const
	{
		return listeners;
	}

	//Get the connector
	TradeBookingConnector<T>* GetConnector()
	{
		return connector;
	}

	//Get the listener to the execution
	TradeBookingExecutionListener<T>* GetExecutionListener()
	{
		return execution_listener;
	}

	// Book the trade
	void BookTrade(const Trade<T>& trade)
	{
		//Notify all the listeners
		for (auto i = listeners.begin(); i != listeners.end(); i++)
		{
			(*i)->ProcessAdd(trade);
		}
	}


};

template<typename T>
Trade<T>::Trade(const T &_product, string _tradeId, double _price, string _book, long _quantity, Side _side) :
  product(_product)
{
  tradeId = _tradeId;
  price = _price;
  book = _book;
  quantity = _quantity;
  side = _side;
}

template<typename T>
const T& Trade<T>::GetProduct() const
{
  return product;
}

template<typename T>
const string& Trade<T>::GetTradeId() const
{
  return tradeId;
}

template<typename T>
double Trade<T>::GetPrice() const
{
  return price;
}

template<typename T>
const string& Trade<T>::GetBook() const
{
  return book;
}

template<typename T>
long Trade<T>::GetQuantity() const
{
  return quantity;
}

template<typename T>
Side Trade<T>::GetSide() const
{
  return side;
}

/*
* TradeBookingConnector class, read data from trade.txt through subscriber
*/

template<typename T>
class TradeBookingConnector : public Connector<Trade<T>>
{
private:
	TradeBookingService<T>* service;
public:
	//Ctor and Dtor
	TradeBookingConnector(TradeBookingService<T>* s)
	{
		service = s;
	}
	~TradeBookingConnector() = default;

	//Publish Data
	void Publish(Trade<T>& data) {}

	//Subscribe data
	void Subscribe(ifstream& data)
	{
		string line;
		
		while (getline(data, line))
		{
			//cout << line << endl;
			string ele = "";
			vector<string> elements;
			for (int i = 0; i < line.size(); i++)
			{
				if (line[i] == ',')
				{
					elements.push_back(ele);
					ele = "";
				}
				else
				{
					ele.push_back(line[i]);
				}
			}
			elements.push_back(ele);
			string cusip = elements[0];
			string tradeid = elements[1];
			double price = FractionaltoPrice(elements[2]);
			string book = elements[3];
			long quantity = stol(elements[4]);
			Side side=BUY;
			if (elements[5] == "SELL") side = SELL;
			//std::cout << "end" << std::endl;
			T product = MakeBond(cusip);
			//std::cout << product.GetProductId() << std::endl;
			Trade<T> trade(product, tradeid, price, book, quantity, side);
			//std::cout << tradeid << std::endl;
			//Notify other services
			service->OnMessage(trade);
			//std::cout << tradeid << "," << product.GetProductId() << "," << price << "," << book << "," << quantity << "," << side<<endl;
			
		}
	}
};

/*
* Listener to execution class
*/
template<typename T>
class TradeBookingExecutionListener : public ServiceListener<ExecutionOrder<T>>
{
private:
	TradeBookingService<T>* service;
	long tradecount;
public:
	//Ctor and Dtor
	TradeBookingExecutionListener(TradeBookingService<T>* s)
	{
		service = s;
	}
	~TradeBookingExecutionListener(){}

	// Listener callback to process an add event to the Service
	void ProcessAdd(ExecutionOrder<T>& data)
	{
		//std::cout << "Start" << endl;
		tradecount++;
		T product = data.GetProduct();
		PricingSide pside = data.GetPricingSide();
		string orderId = data.GetOrderId();
		double price = data.GetPrice();
		long quantity = data.GetQuantity();
		Side side;
		if (pside == BID) side = SELL;
		else if (pside == OFFER) side = BUY;
		string book;
		if (tradecount % 3 == 1) book = "TRSY1";
		else if(tradecount % 3 == 2) book = "TRSY2";
		else book = "TRSY3";
		Trade<T> trade(product, orderId, price, book, quantity, side);
		service->OnMessage(trade);
		//std::cout << orderId << "," << product.GetProductId() << "," << price << "," << book << "," << quantity << "," << side<<endl;
	}

	// Listener callback to process a remove event to the Service
	void ProcessRemove(ExecutionOrder<T>& data) {}

	// Listener callback to process an update event to the Service
	void ProcessUpdate(ExecutionOrder<T>& data) {}
};





#endif