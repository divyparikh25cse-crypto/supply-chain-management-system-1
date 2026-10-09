#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <limits>
#include <utility>

using namespace std;


// C++11-compatible replacement for std::quoted (C++14).
class QuotedToken {
public:
    const string& value;
    explicit QuotedToken(const string& s) : value(s) {}
};
QuotedToken quoteToken(const string& s) { return QuotedToken(s); }

ostream& operator<<(ostream& out, const QuotedToken& q) {
    out << '"';
    for (char ch : q.value) {
        if (ch == '"' || ch == '\\') out << '\\';
        out << ch;
    }
    out << '"';
    return out;
}
istream& operator>>(istream& in, const QuotedToken& q) {
    string& target = const_cast<string&>(q.value);
    target.clear();
    in >> ws;
    if (in.peek() != '"') return in >> target;
    in.get();
    char ch;
    while (in.get(ch)) {
        if (ch == '"') break;
        if (ch == '\\' && in.peek() != char_traits<char>::eof()) {
            in.get(ch);
        }
        target += ch;
    }
    return in;
}

// ---------- Utility functions ----------
int readInt(const string& prompt) {
    int value;
    while (true) {
        cout << prompt;
        if (cin >> value) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return value;
        }
        cout << "Invalid input. Enter a whole number.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

double readDouble(const string& prompt) {
    double value;
    while (true) {
        cout << prompt;
        if (cin >> value && value >= 0) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return value;
        }
        cout << "Invalid input. Enter a non-negative number.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

string readLine(const string& prompt) {
    string value;
    cout << prompt;
    getline(cin, value);
    return value;
}

// ---------- Base class: runtime polymorphism ----------
class Person {
protected:
    int id;
    string name;
    string contact;

public:
    Person() : id(0), name(""), contact("") {}
    Person(int i, const string& n, const string& c)
        : id(i), name(n), contact(c) {}
    virtual ~Person() = default; // virtual destructor for base-class safety

    int getId() const { return id; }
    string getName() const { return name; }
    string getContact() const { return contact; }

    virtual string role() const = 0;
    virtual void display() const {
        cout << left << setw(8) << id << setw(24) << name
             << setw(22) << contact << setw(14) << role() << '\n';
    }

    virtual void save(ofstream& out) const {
        out << id << ' ' << quoteToken(name) << ' ' << quoteToken(contact) << '\n';
    }
};

class Customer : public Person {
public:
    Customer() = default;
    Customer(int i, const string& n, const string& c) : Person(i, n, c) {}
    ~Customer() override = default;

    string role() const override { return "Customer"; }

    void save(ofstream& out) const override {
        Person::save(out);
    }
};

class Supplier : public Person {
private:
    string company;

public:
    Supplier() : Person(), company("") {}
    Supplier(int i, const string& n, const string& c, const string& co)
        : Person(i, n, c), company(co) {}
    ~Supplier() override = default;

    string role() const override { return "Supplier"; }
    string getCompany() const { return company; }

    void display() const override {
        Person::display();
        cout << "         Company: " << company << '\n';
    }

    void save(ofstream& out) const override {
        Person::save(out);
        out << quoteToken(company) << '\n';
    }
};

class Product {
private:
    int id;
    string name;
    double price;
    int stock;
    int supplierId;

public:
    Product() : id(0), name(""), price(0), stock(0), supplierId(0) {}
    Product(int i, const string& n, double p, int s, int supId)
        : id(i), name(n), price(p), stock(s), supplierId(supId) {}
    ~Product() = default;

    int getId() const { return id; }
    string getName() const { return name; }
    double getPrice() const { return price; }
    int getStock() const { return stock; }
    int getSupplierId() const { return supplierId; }

    void setName(const string& n) { name = n; }
    void setPrice(double p) { price = p; }
    void setSupplierId(int sid) { supplierId = sid; }
    void addStock(int amount) { if (amount > 0) stock += amount; }
    bool reduceStock(int amount) {
        if (amount <= 0 || amount > stock) return false;
        stock -= amount;
        return true;
    }

    void display() const {
        cout << left << setw(8) << id << setw(25) << name
             << right << setw(12) << fixed << setprecision(2) << price
             << setw(10) << stock << setw(12) << supplierId << '\n';
    }

    void save(ofstream& out) const {
        out << id << ' ' << quoteToken(name) << ' ' << price << ' '
            << stock << ' ' << supplierId << '\n';
    }
};

class Order {
private:
    int id;
    int customerId;
    int productId;
    int quantity;
    double total;

public:
    Order() : id(0), customerId(0), productId(0), quantity(0), total(0) {}
    Order(int i, int cid, int pid, int q, double t)
        : id(i), customerId(cid), productId(pid), quantity(q), total(t) {}
    ~Order() = default;

    int getId() const { return id; }
    int getCustomerId() const { return customerId; }
    int getProductId() const { return productId; }
    int getQuantity() const { return quantity; }
    double getTotal() const { return total; }

    void display() const {
        cout << left << setw(8) << id << setw(12) << customerId
             << setw(12) << productId << setw(10) << quantity
             << right << fixed << setprecision(2) << total << '\n';
    }

    void save(ofstream& out) const {
        out << id << ' ' << customerId << ' ' << productId << ' '
            << quantity << ' ' << total << '\n';
    }
};

// ---------- System manager ----------
class SupplyChainSystem {
private:
    // unique_ptr demonstrates dynamic memory allocation with automatic cleanup.
    vector<unique_ptr<Product>> products;
    vector<unique_ptr<Customer>> customers;
    vector<unique_ptr<Supplier>> suppliers;
    vector<unique_ptr<Order>> orders;

    int nextOrderId = 1;
    const string productsFile = "products.txt";
    const string customersFile = "customers.txt";
    const string suppliersFile = "suppliers.txt";
    const string ordersFile = "orders.txt";

    Product* findProduct(int id) {
        for (auto& p : products) if (p->getId() == id) return p.get();
        return nullptr;
    }
    Customer* findCustomer(int id) {
        for (auto& c : customers) if (c->getId() == id) return c.get();
        return nullptr;
    }
    Supplier* findSupplier(int id) {
        for (auto& s : suppliers) if (s->getId() == id) return s.get();
        return nullptr;
    }
    bool productIdExists(int id) const {
        for (const auto& p : products) if (p->getId() == id) return true;
        return false;
    }
    bool customerIdExists(int id) const {
        for (const auto& c : customers) if (c->getId() == id) return true;
        return false;
    }
    bool supplierIdExists(int id) const {
        for (const auto& s : suppliers) if (s->getId() == id) return true;
        return false;
    }

public:
    SupplyChainSystem() { loadAll(); }
    ~SupplyChainSystem() { saveAll(); }

    void addProduct() {
        int id = readInt("Product ID: ");
        if (id <= 0 || productIdExists(id)) {
            cout << "Invalid or duplicate product ID.\n";
            return;
        }
        string name = readLine("Product name: ");
        double price = readDouble("Unit price: ");
        int stock = readInt("Opening stock (0 or more): ");
        if (stock < 0) {
            cout << "Stock cannot be negative.\n";
            return;
        }
        int sid = readInt("Supplier ID (0 if unknown): ");
        if (sid != 0 && !supplierIdExists(sid)) {
            cout << "Supplier not found. Use 0 or add the supplier first.\n";
            return;
        }
        products.push_back(unique_ptr<Product>(new Product(id, name, price, stock, sid)));
        saveAll();
        cout << "Product added successfully.\n";
    }

    void showProducts() const {
        if (products.empty()) { cout << "No products found.\n"; return; }
        cout << left << setw(8) << "ID" << setw(25) << "Name"
             << right << setw(12) << "Price" << setw(10) << "Stock"
             << setw(12) << "Supplier" << '\n';
        cout << string(67, '-') << '\n';
        for (const auto& p : products) p->display();
    }

    void searchProduct() {
        int id = readInt("Enter product ID to search: ");
        Product* p = findProduct(id);
        if (!p) { cout << "Product not found.\n"; return; }
        cout << left << setw(8) << "ID" << setw(25) << "Name"
             << right << setw(12) << "Price" << setw(10) << "Stock"
             << setw(12) << "Supplier" << '\n';
        p->display();
    }

    void updateProduct() {
        int id = readInt("Enter product ID to update: ");
        Product* p = findProduct(id);
        if (!p) { cout << "Product not found.\n"; return; }
        cout << "1. Update name\n2. Update price\n3. Update supplier ID\n";
        int choice = readInt("Choice: ");
        if (choice == 1) {
            p->setName(readLine("New name: "));
        } else if (choice == 2) {
            p->setPrice(readDouble("New price: "));
        } else if (choice == 3) {
            int sid = readInt("New supplier ID (0 if unknown): ");
            if (sid != 0 && !supplierIdExists(sid)) {
                cout << "Supplier not found; no changes made.\n";
                return;
            }
            p->setSupplierId(sid);
        } else {
            cout << "Invalid choice.\n";
            return;
        }
        saveAll();
        cout << "Product updated.\n";
    }

    void deleteProduct() {
        int id = readInt("Enter product ID to delete: ");
        auto it = find_if(products.begin(), products.end(),
            [id](const unique_ptr<Product>& p) { return p->getId() == id; });
        if (it == products.end()) { cout << "Product not found.\n"; return; }
        for (const auto& o : orders) {
            if (o->getProductId() == id) {
                cout << "Cannot delete: this product appears in order history.\n";
                return;
            }
        }
        products.erase(it);
        saveAll();
        cout << "Product deleted.\n";
    }

    void addStock() {
        int id = readInt("Product ID: ");
        Product* p = findProduct(id);
        if (!p) { cout << "Product not found.\n"; return; }
        int amount = readInt("Quantity to add: ");
        if (amount <= 0) { cout << "Quantity must be greater than zero.\n"; return; }
        p->addStock(amount);
        saveAll();
        cout << "Stock updated. New stock: " << p->getStock() << '\n';
    }

    void addCustomer() {
        int id = readInt("Customer ID: ");
        if (id <= 0 || customerIdExists(id)) {
            cout << "Invalid or duplicate customer ID.\n"; return;
        }
        string name = readLine("Customer name: ");
        string contact = readLine("Contact/email: ");
        customers.push_back(unique_ptr<Customer>(new Customer(id, name, contact)));
        saveAll();
        cout << "Customer added successfully.\n";
    }

    void manageCustomers() {
        cout << "1. Add customer\n2. List customers\n3. Search customer\n";
        cout << "4. Update customer\n5. Delete customer\n";
        int choice = readInt("Choice: ");
        if (choice == 1) { addCustomer(); return; }
        if (choice == 2) {
            if (customers.empty()) { cout << "No customers found.\n"; return; }
            cout << left << setw(8) << "ID" << setw(24) << "Name"
                 << setw(22) << "Contact" << "Role\n";
            for (const auto& c : customers) c->display();
            return;
        }
        int id = readInt("Customer ID: ");
        Customer* c = findCustomer(id);
        if (choice == 3) {
            if (c) c->display(); else cout << "Customer not found.\n";
        } else if (choice == 4) {
            if (!c) { cout << "Customer not found.\n"; return; }
            // Customer fields are intentionally read-only in this minimal model.
            cout << "Update is not available for customer fields in this version.\n";
        } else if (choice == 5) {
            if (!c) { cout << "Customer not found.\n"; return; }
            for (const auto& o : orders) if (o->getCustomerId() == id) {
                cout << "Cannot delete customer with order history.\n"; return;
            }
            customers.erase(remove_if(customers.begin(), customers.end(),
                [id](const unique_ptr<Customer>& item) { return item->getId() == id; }),
                customers.end());
            saveAll();
            cout << "Customer deleted.\n";
        } else cout << "Invalid choice.\n";
    }

    void addSupplier() {
        int id = readInt("Supplier ID: ");
        if (id <= 0 || supplierIdExists(id)) {
            cout << "Invalid or duplicate supplier ID.\n"; return;
        }
        string name = readLine("Contact person: ");
        string contact = readLine("Contact/email/phone: ");
        string company = readLine("Company name: ");
        suppliers.push_back(unique_ptr<Supplier>(new Supplier(id, name, contact, company)));
        saveAll();
        cout << "Supplier added successfully.\n";
    }

    void listSuppliers() const {
        if (suppliers.empty()) { cout << "No suppliers found.\n"; return; }
        cout << left << setw(8) << "ID" << setw(24) << "Name"
             << setw(22) << "Contact" << "Role\n";
        for (const auto& s : suppliers) s->display();
    }

    void placeOrder() {
        if (products.empty() || customers.empty()) {
            cout << "Add at least one product and one customer before ordering.\n";
            return;
        }
        int cid = readInt("Customer ID: ");
        if (!findCustomer(cid)) { cout << "Customer not found.\n"; return; }
        int pid = readInt("Product ID: ");
        Product* p = findProduct(pid);
        if (!p) { cout << "Product not found.\n"; return; }
        int qty = readInt("Quantity: ");
        if (qty <= 0) { cout << "Quantity must be greater than zero.\n"; return; }
        if (qty > p->getStock()) {
            cout << "Insufficient stock. Available: " << p->getStock() << '\n';
            return;
        }

        double total = qty * p->getPrice();
        if (!p->reduceStock(qty)) {
            cout << "Could not reserve stock; order cancelled.\n";
            return;
        }
        orders.push_back(unique_ptr<Order>(new Order(nextOrderId++, cid, pid, qty, total)));
        saveAll();
        cout << "Order placed successfully. Order ID: " << orders.back()->getId()
             << "\nTotal: " << fixed << setprecision(2) << total
             << "\nRemaining stock: " << p->getStock() << '\n';
    }

    void listOrders() const {
        if (orders.empty()) { cout << "No orders found.\n"; return; }
        cout << left << setw(8) << "OrderID" << setw(12) << "Customer"
             << setw(12) << "Product" << setw(10) << "Quantity" << "Total\n";
        for (const auto& o : orders) o->display();
    }

    void saveAll() const {
        {
            ofstream out(productsFile);
            for (const auto& p : products) p->save(out);
        }
        {
            ofstream out(customersFile);
            for (const auto& c : customers) c->save(out);
        }
        {
            ofstream out(suppliersFile);
            for (const auto& s : suppliers) s->save(out);
        }
        {
            ofstream out(ordersFile);
            for (const auto& o : orders) o->save(out);
        }
    }

    void loadAll() {
        {
            ifstream in(productsFile);
            int id, stock, sid;
            double price;
            string name;
            while (in >> id >> quoteToken(name) >> price >> stock >> sid)
                products.push_back(unique_ptr<Product>(new Product(id, name, price, stock, sid)));
        }
        {
            ifstream in(customersFile);
            int id;
            string name, contact;
            while (in >> id >> quoteToken(name) >> quoteToken(contact))
                customers.push_back(unique_ptr<Customer>(new Customer(id, name, contact)));
        }
        {
            ifstream in(suppliersFile);
            int id;
            string name, contact, company;
            while (in >> id >> quoteToken(name) >> quoteToken(contact) >> quoteToken(company))
                suppliers.push_back(unique_ptr<Supplier>(new Supplier(id, name, contact, company)));
        }
        {
            ifstream in(ordersFile);
            int id, cid, pid, qty;
            double total;
            while (in >> id >> cid >> pid >> qty >> total) {
                orders.push_back(unique_ptr<Order>(new Order(id, cid, pid, qty, total)));
                nextOrderId = max(nextOrderId, id + 1);
            }
        }
    }

    void run() {
        int choice;
        do {
            cout << "\n========== SUPPLY CHAIN MANAGEMENT SYSTEM ==========\n"
                 << "1. Add product\n"
                 << "2. List products\n"
                 << "3. Search product\n"
                 << "4. Update product\n"
                 << "5. Delete product\n"
                 << "6. Add stock\n"
                 << "7. Customer management\n"
                 << "8. Add supplier\n"
                 << "9. List suppliers\n"
                 << "10. Place order (main transaction)\n"
                 << "11. List order history\n"
                 << "12. Save data\n"
                 << "0. Exit\n";
            choice = readInt("Enter choice: ");
            switch (choice) {
                case 1: addProduct(); break;
                case 2: showProducts(); break;
                case 3: searchProduct(); break;
                case 4: updateProduct(); break;
                case 5: deleteProduct(); break;
                case 6: addStock(); break;
                case 7: manageCustomers(); break;
                case 8: addSupplier(); break;
                case 9: listSuppliers(); break;
                case 10: placeOrder(); break;
                case 11: listOrders(); break;
                case 12: saveAll(); cout << "Data saved.\n"; break;
                case 0: saveAll(); cout << "Data saved. Goodbye!\n"; break;
                default: cout << "Invalid menu choice.\n";
            }
        } while (choice != 0);
    }
};

int main() {
    SupplyChainSystem system;
    system.run();
    return 0;
}
