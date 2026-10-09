# UML Diagrams

The Mermaid diagrams below can be rendered in Mermaid Live Editor or compatible Markdown viewers.

## 1. Use Case Diagram
```mermaid
flowchart LR
    U[User / Store Manager]
    U --> A((Add Product))
    U --> B((Search / List Product))
    U --> C((Update Product))
    U --> D((Delete Product))
    U --> E((Add Stock))
    U --> F((Manage Customers))
    U --> G((Manage Suppliers))
    U --> H((Place Order))
    U --> I((View Order History))
    U --> J((Save Data))
```

## 2. Class Diagram
```mermaid
classDiagram
    class Person {
      <<abstract>>
      #int id
      #string name
      #string contact
      +getId() int
      +role()* string
      +display() void
      +save(ofstream&) void
      +~Person()
    }
    class Customer {
      +role() string
      +save(ofstream&) void
    }
    class Supplier {
      -string company
      +role() string
      +display() void
      +save(ofstream&) void
    }
    class Product {
      -int id
      -string name
      -double price
      -int stock
      -int supplierId
      +addStock(int) void
      +reduceStock(int) bool
      +display() void
      +save(ofstream&) void
    }
    class Order {
      -int id
      -int customerId
      -int productId
      -int quantity
      -double total
      +display() void
      +save(ofstream&) void
    }
    class SupplyChainSystem {
      -vector~unique_ptr~Product~~ products
      -vector~unique_ptr~Customer~~ customers
      -vector~unique_ptr~Supplier~~ suppliers
      -vector~unique_ptr~Order~~ orders
      +addProduct() void
      +searchProduct() void
      +updateProduct() void
      +deleteProduct() void
      +placeOrder() void
      +saveAll() void
      +loadAll() void
    }
    Person <|-- Customer
    Person <|-- Supplier
    SupplyChainSystem o-- Product
    SupplyChainSystem o-- Customer
    SupplyChainSystem o-- Supplier
    SupplyChainSystem o-- Order
```

## 3. Sequence Diagram — Add Product
```mermaid
sequenceDiagram
    actor User
    participant UI as SupplyChainSystem
    participant P as Product
    participant File as products.txt
    User->>UI: Choose Add Product
    UI->>User: Ask for ID, name, price, stock, supplier
    User-->>UI: Enter product details
    UI->>UI: Validate unique ID and supplier
    UI->>P: Create Product
    UI->>File: Save all products
    UI-->>User: Product added successfully
```

## 4. Sequence Diagram — Place Order
```mermaid
sequenceDiagram
    actor User
    participant UI as SupplyChainSystem
    participant C as Customer
    participant P as Product
    participant O as Order
    participant File as orders.txt / products.txt
    User->>UI: Choose Place Order
    UI->>User: Request customer, product, quantity
    User-->>UI: Submit order details
    UI->>C: Find customer
    UI->>P: Find product and check stock
    alt Customer/product valid and stock sufficient
        UI->>P: reduceStock(quantity)
        UI->>O: Create order with total
        UI->>File: Save order and updated stock
        UI-->>User: Confirm order and total
    else Invalid ID or insufficient stock
        UI-->>User: Reject order with reason
    end
```
