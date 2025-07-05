#include <iostream>
#include <vector>
#include <string>
#include <ctime>
#include <algorithm>

using namespace std;

class Exception
{
public:
    string message;
    Exception(const string& msg) : message(msg) {}
};

class Product
{
public:
    string name;
    double price;
    int quantity;
    bool isExpired;
    bool isShippable;
    time_t expiryDate;
    double weight;

    Product(const string& name, double price, int quantity)
        : name(name), price(price), quantity(quantity),
        isExpired(false), isShippable(false), expiryDate(0), weight(0) {}

    void setExpired(time_t expiry, double itemWeight)
    {
        isExpired = true;
        expiryDate = expiry;
        weight = itemWeight;
    }

    void setShippable(double itemWeight)
    {
        isShippable = true;
        weight = itemWeight;
    }

    bool checkExpired() const
    {
        if (!isExpired) return false;
        time_t now = time(nullptr);
        return now > expiryDate;
    }

    bool checkAvailable(int requested) const
    {
        return quantity >= requested && !checkExpired();
    }

    void reduceQuantity(int amount)
    {
        if (amount > quantity)
        {
            throw Exception("Not enough stock for " + name);
        }
        quantity -= amount;
    }
};

class Customer
{
public:
    string name;
    double balance;

    Customer(const string& name, double balance) : name(name), balance(balance) {}

    void Insufficient(double amount)
    {
        if (amount > balance)
        {
            throw Exception("Insufficient balance for " + name);
        }
        balance -= amount;
    }
};

class CartItem
{
public:
    Product* product;
    int quantity;

    CartItem(Product* p, int qty) : product(p), quantity(qty) {}

    double totalPrice() const
    {
        return product->price * quantity;
    }
};

class ShoppingCart
{
public:
    vector<CartItem> items;

    void addItem(Product* product, int quantity)
    {
        if (!product->checkAvailable(quantity))
        {
            throw Exception("Product " + product->name + " not available");
        }
        items.emplace_back(product, quantity);
    }

    bool empty() const
    {
        return items.empty();
    }

    double calculateSubtotal() const
    {
        double total = 0;
        for (int i = 0; i < items.size(); i++)
        {
            total += items[i].totalPrice();
        }
        return total;
    }

    vector<Product*> Shipping() const
    {
        vector<Product*> shippable;
        for (int i = 0; i < items.size(); i++)
        {
            if (items[i].product->isShippable)
            {
                shippable.push_back(items[i].product);
            }
        }
        return shippable;
    }
};

class ShippingService
{
public:
    static void shipItems(const vector<Product*>& items)
    {
        if (items.empty()) return;

        cout << "** Shipment notice **" << '\n';
        double totalWeight = 0;

        for (int i = 0; i < items.size(); i++) {
            cout << "1x " << items[i]->name << "    " << items[i]->weight << "g" << '\n';
            totalWeight += items[i]->weight;
        }

        cout << "Total package weight " << (totalWeight / 1000) << "kg" << '\n' << '\n';
    }
};

class CheckoutService
{
public:
    static void processCheckout(Customer& customer, ShoppingCart& cart)
    {
        if (cart.empty())
        {
            throw Exception("Cannot checkout with empty cart");
        }

        double shipping = 0;
        auto shippable = cart.Shipping();
        for (int i = 0; i < shippable.size(); i++)
        {
            shipping += shippable[i]->weight;
        }
        shipping = (shipping / 1000) * 10;

        double subtotal = cart.calculateSubtotal();
        double total = subtotal + shipping;

        customer.Insufficient(total);

        for (int i = 0; i < cart.items.size(); i++)
        {
            cart.items[i].product->reduceQuantity(cart.items[i].quantity);
        }

        ShippingService::shipItems(shippable);
        printReceipt(cart, subtotal, shipping, total, customer);
    }

private:
    static void printReceipt(const ShoppingCart& cart, double subtotal,
        double shipping, double total, const Customer& customer)
    {
        cout << "** Checkout receipt **" << '\n';
        for (int i = 0; i < cart.items.size(); i++)
        {
            cout << cart.items[i].quantity << "x " << cart.items[i].product->name << "    "
                << cart.items[i].totalPrice() << '\n';
        }

        cout << "---" << '\n';
        cout << "Subtotal    " << subtotal << '\n';
        cout << "Shipping    " << shipping << '\n';
        cout << "Amount    " << total << '\n';
        cout << "Remaining balance: " << customer.balance << '\n';
    }
};

time_t createFutureDate(int daysInFuture)
{
    time_t now = time(nullptr);
    return now + (daysInFuture * 24 * 60 * 60);
}

int main() {


    try {
        Product cheese("Cheese", 100.0, 10);
        cheese.setExpired(createFutureDate(7), 200); // to test Expiration change 7 to any negative number like (-1)
        cheese.setShippable(1000);

        Product biscuits("Biscuits", 150.0, 3);
        biscuits.setExpired(createFutureDate(14), 700);

        Product Meat("Meat", 150.0, 5);
        biscuits.setExpired(createFutureDate(14), 700);

        Product tv("TV", 15000.0, 3);
        tv.setShippable(5000);

        Product scratchCard("Mobile Scratch Card", 50.0, 100);

        Customer customer("John Doe", 20000); // to test Insufficient balance change it to a number less than the total

        ShoppingCart cart;
        cart.addItem(&cheese, 2);
        cart.addItem(&biscuits, 2); // to test stock out change any of this items to a number more than the stock ( like : make number 2 in biscuits to 5)
        cart.addItem(&tv, 1);
        cart.addItem(&scratchCard, 3);
        cart.addItem(&Meat, 2);

        CheckoutService::processCheckout(customer, cart);

    }
    catch (const Exception& e) {
        cerr << "Error: " << e.message << '\n';
        return 1;
    }

    return 0;
}