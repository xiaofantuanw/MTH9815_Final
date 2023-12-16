/**
 * marketdataservice.hpp
 * Defines the data types and Service for order book market data.
 *
 * @author Breman Thuraisingham
 */
#ifndef MARKET_DATA_SERVICE_HPP
#define MARKET_DATA_SERVICE_HPP

#include <string>
#include <vector>
#include "soa.hpp"

using namespace std;

// Side for market data
enum PricingSide { BID, OFFER };

/**
 * A market data order with price, quantity, and side.
 */
class Order
{

public:

  // ctor for an order
	Order() {}
	Order(double _price, long _quantity, PricingSide _side);

  // Get the price on the order
	double GetPrice() const;

  // Get the quantity on the order
	long GetQuantity() const;

  // Get the side on the order
	PricingSide GetSide() const;

private:
	double price;
	long quantity;
	PricingSide side;

};

/**
 * Class representing a bid and offer order
 */
class BidOffer
{

public:

  // ctor for bid/offer
	BidOffer() {}
	BidOffer(const Order &_bidOrder, const Order &_offerOrder);

  // Get the bid order
	const Order& GetBidOrder() const;

  // Get the offer order
	const Order& GetOfferOrder() const;

private:
	Order bidOrder;
	Order offerOrder;

};

/**
 * Order book with a bid and offer stack.
 * Type T is the product type.
 */
template<typename T>
class OrderBook
{

public:

  // ctor for the order book
	OrderBook() {}
	OrderBook(const T &_product, const vector<Order> &_bidStack, const vector<Order> &_offerStack);

  // Get the product
	const T& GetProduct() const;

  // Get the bid stack
	const vector<Order>& GetBidStack() const;

  // Get the offer stack
	const vector<Order>& GetOfferStack() const;

	//Get the best bid offer order
	BidOffer GetBidOffer() const
	{
		double bid = -100;
		double offer = 10000;
		Order bidorder;
		Order offerorder;
		for (auto b = bidStack.begin(); b != bidStack.end(); b++)
		{
			double price = b->GetPrice();
			if (price > bid)
			{
				bid = price;
				bidorder = (*b);
			}
		}
		for (auto o = bidStack.begin(); o != bidStack.end(); o++)
		{
			double price = o->GetPrice();
			if (price <offer)
			{
				offer = price;
				offerorder = (*o);
			}
		}
		return BidOffer(bidorder, offerorder);
	}

private:
  T product;
  vector<Order> bidStack;
  vector<Order> offerStack;

};

/*
* Pre declaration of connector to read data
*/
template<typename T>
class MarketDataConnector;

/**
 * Market Data Service which distributes market data
 * Keyed on product identifier.
 * Type T is the product type.
 */
template<typename T>
class MarketDataService : public Service<string,OrderBook <T> >
{
private:
	map<string, OrderBook<T>> orderbookmap;
	vector<ServiceListener<OrderBook<T>>*> listeners;
	MarketDataConnector<T>* connector;

public:
	//Ctor and Dtor
	MarketDataService()
	{
		orderbookmap= map<string, OrderBook<T>>();
		listeners = vector<ServiceListener<OrderBook<T>>*>();
		connector = new MarketDataConnector<T>(this);
	}
	~MarketDataService() = default;

	// Get data on our service given a key
	virtual OrderBook<T>& GetData(string key)
	{
		return orderbookmap[key];
	}

	// The callback that a Connector should invoke for any new or updated data
	virtual void OnMessage(OrderBook<T>& data)
	{
		string key = data.GetProduct().GetProductId();
		orderbookmap[key] = data;
		//Call all the listeners
		for (auto i = listeners.begin(); i != listeners.end(); i++)
		{
			(*i)->ProcessAdd(data);
		}
	}

	// Add a listener to the Service for callbacks on add, remove, and update events for data to the Service
	virtual void AddListener(ServiceListener<OrderBook<T>>* listener)
	{
		listeners.push_back(listener);
	}

	// Get all listeners on the Service
	virtual const vector<ServiceListener<OrderBook<T>>*>& GetListeners() const
	{
		return listeners;
	}

	// Get the connector of the service
	virtual MarketDataConnector<T>* GetConnector()
	{
		return connector;
	}

  // Get the best bid/offer order
	BidOffer GetBestBidOffer(const string& productId)
	{
		return orderbookmap[productId].GetBidOffer();
	}

  // Aggregate the order book
	OrderBook<T> AggregateDepth(const string& productId)
	{
		auto product = orderbookmap[productId].GetProduct();
		vector<Order> bidold = orderbookmap[productId].GetBidStack();
		vector<Order> offerold = orderbookmap[productId].GetOfferStack();
		map<double, long> aggregatedbid, aggregatedoffer;

		for (auto b = bidold.begin(); b != bidold.end(); b++)
		{
			double price = b->GetPrice();
			long quantity = b->GetQuantity();
			aggregatedbid[price] += quantity;
		}
		for (auto o = offerold.begin(); o != offerold.end(); o++)
		{
			double price = o->GetPrice();
			long quantity = o->GetQuantity();
			aggregatedoffer[price] += quantity;
		}

		//Build a new bidstack and offerstack using the data
		vector<Order> bidnew, offernew;
		for (auto i = aggregatedbid.begin(); i != aggregatedbid.end(); i++)
		{
			Order order = Order(i->first, i->second, BID);
			bidnew.push_back(order);
		}
		for (auto i = aggregatedoffer.begin(); i != aggregatedoffer.end(); i++)
		{
			Order order = Order(i->first, i->second, OFFER);
			offernew.push_back(order);
		}
		
		return OrderBook<T>(product, bidnew, offernew);
	}

};

Order::Order(double _price, long _quantity, PricingSide _side)
{
  price = _price;
  quantity = _quantity;
  side = _side;
}

double Order::GetPrice() const
{
  return price;
}
 
long Order::GetQuantity() const
{
  return quantity;
}
 
PricingSide Order::GetSide() const
{
  return side;
}

BidOffer::BidOffer(const Order &_bidOrder, const Order &_offerOrder) :
  bidOrder(_bidOrder), offerOrder(_offerOrder)
{
}

const Order& BidOffer::GetBidOrder() const
{
  return bidOrder;
}

const Order& BidOffer::GetOfferOrder() const
{
  return offerOrder;
}

template<typename T>
OrderBook<T>::OrderBook(const T &_product, const vector<Order> &_bidStack, const vector<Order> &_offerStack) :
  product(_product), bidStack(_bidStack), offerStack(_offerStack)
{
}

template<typename T>
const T& OrderBook<T>::GetProduct() const
{
  return product;
}

template<typename T>
const vector<Order>& OrderBook<T>::GetBidStack() const
{
  return bidStack;
}

template<typename T>
const vector<Order>& OrderBook<T>::GetOfferStack() const
{
  return offerStack;
}


/*
* Market Data connector: subscribing only, read data from txt
*/

template<typename T>
class MarketDataConnector : public Connector<OrderBook<T>>
{
private:
	//The service it is attached to
	MarketDataService<T>* service;
public:
	//Ctor and Dtor
	MarketDataConnector() {}
	MarketDataConnector(MarketDataService<T>* s)
	{
		service = s;
	}
	~MarketDataConnector() = default;

	//Publisher
	void Publish(OrderBook<T>& data) {}

	//Subscriber
	void Subscribe(ifstream& data)
	{
		string line;
		vector<Order> bids, offers;
		int count=0;
		int depth = 10;
		while (getline(data, line))
		{
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
			double price = FractionaltoPrice(elements[1]);
			long quantity = stol(elements[2]);
			PricingSide side = BID;
			if (elements[3] == "OFFER") side = OFFER;
			Order order(price, quantity, side);
			if (elements[3] == "BID")
			{
				bids.push_back(order);
			}
			else if (elements[3] == "OFFER")
			{
				offers.push_back(order);
			}
			count++;
			if (count == depth)
			{
				count = 0;
				T product = MakeBond(cusip);
				OrderBook<T> odb(product, bids, offers);
				service->OnMessage(odb);
				bids = vector<Order>();
				offers = vector<Order>();
			}
		}
	}
};

#endif
