/**
 * inquiryservice.hpp
 * Defines the data types and Service for customer inquiries.
 *
 * @author Breman Thuraisingham
 */
#ifndef INQUIRY_SERVICE_HPP
#define INQUIRY_SERVICE_HPP

#include "soa.hpp"
#include "tradebookingservice.hpp"

// Various inqyury states
enum InquiryState { RECEIVED, QUOTED, DONE, REJECTED, CUSTOMER_REJECTED };

/**
 * Inquiry object modeling a customer inquiry from a client.
 * Type T is the product type.
 */
template<typename T>
class Inquiry
{

public:

  // ctor for an inquiry
	Inquiry() {}
  Inquiry(string _inquiryId, const T &_product, Side _side, long _quantity, double _price, InquiryState _state);

  // Get the inquiry ID
  const string& GetInquiryId() const;

  // Get the product
  const T& GetProduct() const;

  // Get the side on the inquiry
  Side GetSide() const;

  // Get the quantity that the client is inquiring for
  long GetQuantity() const;

  // Get the price that we have responded back with
  double GetPrice();

  // Get the current state on the inquiry
  InquiryState GetState();

  //Set the state
  void SetState(InquiryState s)
  {
	  state = s;
  }

  //Set the price
  void SetPrice(double p)
  {
	  price = p;
  }

  //Output function
  ostream& Output(ostream& file)
  {
	  file << GetProduct().GetProductId() << "," << GetInquiryId() << ",";
	  if (GetSide() == BUY)
	  {
		  file << "BUY,";
	  }
	  else if (GetSide() == SELL)
	  {
		  file << "SELL,";
	  }
	  file << GetQuantity() << "," << GetPrice() << ",";
	  if (GetState() == DONE)
	  {
		  file << "DONE\n";
	  }
	  return file;
  }

private:
  string inquiryId;
  T product;
  Side side;
  long quantity;
  double price;
  InquiryState state;

};


template <typename T>
class InquiryConnector;

/**
 * Service for customer inquirry objects.
 * Keyed on inquiry identifier (NOTE: this is NOT a product identifier since each inquiry must be unique).
 * Type T is the product type.
 */
template<typename T>
class InquiryService : public Service<string,Inquiry <T> >
{

private:
	map<string, Inquiry<T>> inquirymap;
	vector<ServiceListener<Inquiry<T>>*> listeners;
	InquiryConnector<T>* connector;

public:
	//Ctor and Dtor
	InquiryService()
	{
		inquirymap = map<string, Inquiry<T>>();
		listeners = vector<ServiceListener<Inquiry<T>>*>();
		connector = new InquiryConnector<T>(this);
	}
	~InquiryService() = default;

	// Get data on our service given a key
	Inquiry<T>& GetData(string key)
	{
		return inquirymap[key];
	}

	// The callback that a Connector should invoke for any new or updated data
	void OnMessage(Inquiry<T>& data)
	{
		string key = data.GetInquiryId();
		InquiryState state = data.GetState();
		//std::cout << data.GetInquiryId() << "," << data.GetPrice() << "," << data.GetSide() << "," << data.GetQuantity() << "," << data.GetState() << endl;
		if (state == RECEIVED)
		{
			inquirymap[key] = data;
			connector->Publish(data);
		}
		else if (state == QUOTED)
		{
			data.SetState(DONE);
			inquirymap[key] = data;
			for (auto i = listeners.begin(); i != listeners.end(); i++)
			{
				(*i)->ProcessAdd(data);
			}
		}
		//std::cout << data.GetInquiryId() << "," << data.GetPrice() << "," << data.GetSide() << "," << data.GetQuantity() << "," << data.GetState() << endl;
		
	}

	// Add a listener to the Service for callbacks on add, remove, and update events for data to the Service
	void AddListener(ServiceListener<Inquiry<T>>* listener)
	{
		listeners.push_back(listener);
	}

	// Get all listeners on the Service
	const vector<ServiceListener<Inquiry<T>>*>& GetListeners() const
	{
		return listeners;
	}

	// Get the connector of the service
	InquiryConnector<T>* GetConnector()
	{
		return connector;
	}

	// Send a quote back to the client
	void SendQuote(const string& inquiryId, double price)
	{
		Inquiry<T>& inq = inquirymap[inquiryId];
		inq.SetPrice(price);
		for (auto i = listeners.begin(); i != listeners.end(); i++)
		{
			(*i)->ProcessAdd(inq);
		}
	}

	// Reject an inquiry from the client
	void RejectInquiry(const string& inquiryId)
	{
		inquirymap[inquiryId].GetState() = REJECTED;
	}

};

template<typename T>
Inquiry<T>::Inquiry(string _inquiryId, const T &_product, Side _side, long _quantity, double _price, InquiryState _state) :
  product(_product)
{
  inquiryId = _inquiryId;
  side = _side;
  quantity = _quantity;
  price = _price;
  state = _state;
}

template<typename T>
const string& Inquiry<T>::GetInquiryId() const
{
  return inquiryId;
}

template<typename T>
const T& Inquiry<T>::GetProduct() const
{
  return product;
}

template<typename T>
Side Inquiry<T>::GetSide() const
{
  return side;
}

template<typename T>
long Inquiry<T>::GetQuantity() const
{
  return quantity;
}

template<typename T>
double Inquiry<T>::GetPrice()
{
  return price;
}

template<typename T>
InquiryState Inquiry<T>::GetState()
{
  return state;
}


/*
* Connector
*/
template<typename T>
class InquiryConnector :public Connector<Inquiry<T>>
{
private:
	InquiryService<T>* service;
public:
	//Ctor and Dtor
	InquiryConnector(InquiryService<T>* s)
	{
		service = s;
	}
	~InquiryConnector() = default;

	//Publish data 
	void Publish(Inquiry<T>& data)
	{
		InquiryState state = data.GetState();
		if (state == RECEIVED)
		{
			data.SetState(QUOTED);
			this->Subscribe(data);
		}
		
	}

	//Subscribe data- from connector
	void Subscribe(ifstream& data)
	{
		string line;

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
			//std::cout << elements[2] << endl;
			string inquiryid = elements[0];
			string cusip = elements[1];
			Side side = BUY;
			if (elements[2] == "SELL") side = SELL;
			long quantity = stol(elements[3]);
			double price = FractionaltoPrice(elements[4]);
			InquiryState state;
			if (elements[5] == "RECEIVED") state = RECEIVED;
			else if (elements[5] == "QUOTED") state = QUOTED;
			else if (elements[5] == "DONE") state = DONE;
			else if (elements[5] == "REJECTED") state = REJECTED;
			else if (elements[5] == "CUSTOMER_REJECTED") state = CUSTOMER_REJECTED;

			T product = MakeBond(cusip);
			Inquiry<T> inquiry(inquiryid, product, side, quantity, price, state);
			service->OnMessage(inquiry);
			//std::cout << cusip << "," << mid << "," << spread<<std::endl;
		}
	}

	//Subscribe data
	void Subscribe(Inquiry<T>& data)
	{
		service->OnMessage(data);
	}
};



#endif
