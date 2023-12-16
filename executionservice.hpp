/**
 * executionservice.hpp
 * Defines the data types and Service for executions.
 *
 * @author Breman Thuraisingham & Tengxiao Fan
 */
#ifndef EXECUTION_SERVICE_HPP
#define EXECUTION_SERVICE_HPP

#include <string>
#include "soa.hpp"
#include "marketdataservice.hpp"
#include "algoexecutionservice.hpp"

//enum OrderType { FOK, IOC, MARKET, LIMIT, STOP };

//enum Market { BROKERTEC, ESPEED, CME };



/**
 * An execution order that can be placed on an exchange.
 * Type T is the product type.
 */
 /*
template<typename T>
class ExecutionOrder
{

public:

  // ctor for an order
  ExecutionOrder(const T &_product, PricingSide _side, string _orderId, OrderType _orderType, double _price, double _visibleQuantity, double _hiddenQuantity, string _parentOrderId, bool _isChildOrder);

  // Get the product
  const T& GetProduct() const;

  // Get the order ID
  const string& GetOrderId() const;

  // Get the order type on this order
  OrderType GetOrderType() const;

  // Get the price on this order
  double GetPrice() const;

  // Get the visible quantity on this order
  long GetVisibleQuantity() const;

  // Get the hidden quantity
  long GetHiddenQuantity() const;

  // Get the parent order ID
  const string& GetParentOrderId() const;

  // Is child order?
  bool IsChildOrder() const;

private:
  T product;
  PricingSide side;
  string orderId;
  OrderType orderType;
  double price;
  double visibleQuantity;
  double hiddenQuantity;
  string parentOrderId;
  bool isChildOrder;

};
*/

/**
 * Service for executing orders on an exchange.
 * Keyed on product identifier.
 * Type T is the product type.
 */

/*
template<typename T>
ExecutionOrder<T>::ExecutionOrder(const T &_product, PricingSide _side, string _orderId, OrderType _orderType, double _price, double _visibleQuantity, double _hiddenQuantity, string _parentOrderId, bool _isChildOrder) :
  product(_product)
{
  side = _side;
  orderId = _orderId;
  orderType = _orderType;
  price = _price;
  visibleQuantity = _visibleQuantity;
  hiddenQuantity = _hiddenQuantity;
  parentOrderId = _parentOrderId;
  isChildOrder = _isChildOrder;
}

template<typename T>
const T& ExecutionOrder<T>::GetProduct() const
{
  return product;
}

template<typename T>
const string& ExecutionOrder<T>::GetOrderId() const
{
  return orderId;
}

template<typename T>
OrderType ExecutionOrder<T>::GetOrderType() const
{
  return orderType;
}

template<typename T>
double ExecutionOrder<T>::GetPrice() const
{
  return price;
}

template<typename T>
long ExecutionOrder<T>::GetVisibleQuantity() const
{
  return visibleQuantity;
}

template<typename T>
long ExecutionOrder<T>::GetHiddenQuantity() const
{
  return hiddenQuantity;
}

template<typename T>
const string& ExecutionOrder<T>::GetParentOrderId() const
{
  return parentOrderId;
}

template<typename T>
bool ExecutionOrder<T>::IsChildOrder() const
{
  return isChildOrder;
}
*/

template<typename T>
class ExecutionAlgoExecutionListener;

template<typename T>
class ExecutionService : public Service<string, ExecutionOrder <T> >
{

private:
	map<string, ExecutionOrder<T>> executionordermap;
	vector<ServiceListener<ExecutionOrder<T>>*> listeners;
	ExecutionAlgoExecutionListener<T>* AlgoExecutionListener;


public:
	//Ctor and Dtor
	ExecutionService()
	{
		executionordermap = map<string, ExecutionOrder<T>>();
		listeners= vector<ServiceListener<ExecutionOrder<T>>*>();
		AlgoExecutionListener = new ExecutionAlgoExecutionListener<T>(this);
	}
	~ExecutionService() = default;

	// Get data on our service given a key
	ExecutionOrder<T>& GetData(string key)
	{
		return executionordermap[key];
	}

	// The callback that a Connector should invoke for any new or updated data
	void OnMessage(ExecutionOrder<T>& data)
	{
		string key = data.GetProduct().GetProductId();
		executionordermap[key] = data;
		//Call all the listeners
		for (auto i = listeners.begin(); i != listeners.end(); i++)
		{
			(*i)->ProcessAdd(data);
		}
	}

	// Add a listener to the Service for callbacks on add, remove, and update events for data to the Service
	void AddListener(ServiceListener<ExecutionOrder<T>>* listener)
	{
		listeners.push_back(listener);
	}

	// Get all listeners on the Service
	const vector<ServiceListener<ExecutionOrder<T>>*>& GetListeners() const
	{
		return listeners;
	}

	// Get the listener of the service
	ExecutionAlgoExecutionListener<T>* GetAlgoExecutionListener()
	{
		return AlgoExecutionListener;
	}

	// Execute an order on a market
	void ExecuteOrder(ExecutionOrder<T>& order)
	{
		//std::cout << order.GetPrice() <<","<< order.GetQuantity() <<","<< order.GetPricingSide() << endl;
		
		OnMessage(order);
	}

};

template <typename T>
class ExecutionAlgoExecutionListener : public ServiceListener<AlgoExecution<T>>
{
private:

	ExecutionService<T>* service;

public:

	// Connector and Destructor
	ExecutionAlgoExecutionListener(ExecutionService<T>* s)
	{
		service = s;
	}
	~ExecutionAlgoExecutionListener() = default;

	// Listener callback to process an add event to the Service
	void ProcessAdd(AlgoExecution<T>& data)
	{
		ExecutionOrder<T>* order = data.GetExecutionOrder();
		service->ExecuteOrder(*order);
	}

	// Listener callback to process a remove event to the Service
	void ProcessRemove(AlgoExecution<T>& _data) {}

	// Listener callback to process an update event to the Service
	void ProcessUpdate(AlgoExecution<T>& _data) {}
};

#endif
