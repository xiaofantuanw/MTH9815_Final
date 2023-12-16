/**
 * positionservice.hpp
 * Defines the data types and Service for positions.
 *
 * @author Breman Thuraisingham & Tengxiao Fan
 */
#ifndef POSITION_SERVICE_HPP
#define POSITION_SERVICE_HPP

#include <string>
#include <map>
#include "soa.hpp"
#include "tradebookingservice.hpp"

using namespace std;

/**
 * Position class in a particular book.
 * Type T is the product type.
 */
template<typename T>
class Position
{

public:

  // ctor for a position
	Position() {}
	Position(const T &_product);

  // Get the product
  const T& GetProduct() const;

  // Get the position quantity
  long GetPosition(string& book);

  //Add the position by quantity
  void ModifyPosition(string& book, long quantity)
  {
	  positions[book] += quantity;
  }

  // Get the aggregate position
  long GetAggregatePosition();
  T product;

  //Output function
  ostream& Output(ostream& file)
  {
	  string trsy1 = "TRSY1";
	  string trsy2 = "TRSY2";
	  string trsy3 = "TRSY3";
	  file << GetProduct().GetProductId() << ",TRSY1," << GetPosition(trsy1) << ",TRSY2," << GetPosition(trsy2) << ",TRSY3," << GetPosition(trsy3) << ",TOTAL," << GetAggregatePosition() << endl;
	  return file;
  }

private:
  
  map<string,long> positions;

};

template<typename T>
Position<T>::Position(const T& _product) :
	product(_product)
{
}

template<typename T>
const T& Position<T>::GetProduct() const
{
	return product;
}

template<typename T>
long Position<T>::GetPosition(string& book)
{
	return positions[book];
}

template<typename T>
long Position<T>::GetAggregatePosition()
{
	// No-op implementation - should be filled out for implementations
	long aggregateposition = 0;
	for (auto p = positions.begin(); p != positions.end(); p++)
	{
		aggregateposition += p->second;
	}
	return aggregateposition;
}


/*
* Pre declaration for the listener
*/
template <typename T>
class PositionTradeBookingListener;


/**
 * Position Service to manage positions across multiple books and secruties.
 * Keyed on product identifier.
 * Type T is the product type.
 */
template<typename T>
class PositionService : public Service<string,Position <T> >
{
private:
	map<string, Position<T>> positions;
	vector<ServiceListener<Position<T>>*> listeners;
	PositionTradeBookingListener<T>* tradebooking_listener;


public:
	// Constructor and destructor
	PositionService()
	{
		positions = map<string, Position<T>>();
		listeners = vector<ServiceListener<Position<T>>*>();
		tradebooking_listener = new PositionTradeBookingListener<T>(this);

	}
	~PositionService()=default;

	// Get data on our service given a key
	virtual Position<T>& GetData(string key)
	{
		return positions[key];
	}

	// The callback that a Connector should invoke for any new or updated data
	virtual void OnMessage(Position<T>& data)
	{
		string key = data.GetProduct().GetProductId();
		positions[key] = data;
		for (auto i = listeners.begin(); i != listeners.end(); i++)
		{
			(*i)->ProcessAdd(data);
		}
	}

	// Add a listener to the Service for callbacks on add, remove, and update events for data to the Service
	virtual void AddListener(ServiceListener<Position<T>>* listener)
	{
		listeners.push_back(listener);
	}

	// Get all listeners on the Service
	virtual const vector<ServiceListener<Position<T>>*>& GetListeners() const
	{
		return listeners;
	}
	
	//Get the listener of TradeBooking
	PositionTradeBookingListener<T>* GetTradeBookingListener()
	{
		return tradebooking_listener;
	}

	// Add a trade to the service
	//Update the position
	virtual void AddTrade(const Trade<T>& trade)
	{
		T product = trade.GetProduct();
		string id = product.GetProductId();
		double price = trade.GetPrice();
		string book = trade.GetBook();
		long quantity = trade.GetQuantity();
		Side side = trade.GetSide();
		//Position<T> newposition(product);
		Position<T> newposition = positions[id];
		newposition.product = product;
		//cout << newposition.GetProduct().GetProductId() << endl;
		if (side == BUY)
		{
			newposition.ModifyPosition(book,quantity);
		}
		else
		{
			newposition.ModifyPosition(book, -quantity);
		}
		
		positions[id] = newposition;
		//cout << id << endl;
		//cout << positions[id].GetAggregatePosition() << std::endl;
		for (auto i = listeners.begin(); i != listeners.end(); i++)
		{
			(*i)->ProcessAdd(newposition);
		}
	}
};

/*
* This is the listener to trade booking service
*/

template<typename T>
class PositionTradeBookingListener : public ServiceListener<Trade<T>>
{
private:
	PositionService<T>* service;
public:
	// Connector and Destructor
	PositionTradeBookingListener(PositionService<T>* s)
	{
		service = s;
	}
	~PositionTradeBookingListener()=default;

	// Listener callback to process an add event to the Service
	void ProcessAdd(Trade<T>& data)
	{
		service->AddTrade(data);
	}

	// Listener callback to process a remove event to the Service
	void ProcessRemove(Trade<T>& data){}

	// Listener callback to process an update event to the Service
	void ProcessUpdate(Trade<T>& _data){}
};



#endif
