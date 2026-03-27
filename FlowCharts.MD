# AnimeDB Architecture Flowcharts

---
## High-Level Request Flow
```mermaid
flowchart LR
A[Client App] -->|Send Request| B[Network Layer]
B --> C[Message Parser]
C --> D[Request Handler]
D -->|Search / Filter Request| E[Database]
E --> F[filterProcessor]
F --> E
E -->|Return IDs / Data| D
D --> G[Serializer]
G --> H[Message Builder]
H --> B
B -->|Send Response| A
```


---
## Network Stack
```mermaid
flowchart LR

A[Raw Socket] --> B[Byte Stream]
B --> C[Message Header Parser]

C -->|ENCODE_VERSION| D1[Validate Version]
C -->|COM_TYPE| D2[Route Command]
C -->|QUERY_TYPE| D3[Select Handler]
C -->|PAYLOAD_SIZE| D4[Read Payload]
C -->|PAYLOAD| D4

D4 --> E[Payload Bytes]

E --> F[Deserializer]
F --> G[Structured Request]

G --> H[Request Handler]
H --> I[Response Data]

I --> J[Serializer]
J --> K[Payload Bytes]

K --> L[Message Builder]
L --> M[Send via Socket]
```

---
## Serialization
```mermaid
flowchart LR

A[Database] --> B[Database Serializer]

B --> C[FieldMeta Loop]

C --> D[Access Field via Pointer]

D --> E{Field Type?}

E -->|int| F[writeU32]
E -->|string| G[writeString]
E -->|vector| H[writeVector]

F --> I[ByteWriter Buffer]
G --> I
H --> I

I --> J[Payload Bytes]

J --> K[Message Builder]
```

---
## De-Serialization
```mermaid
flowchart LR

A[Incoming Bytes] --> B[ByteReader]

B --> C[FieldMeta Loop]

C --> D{Field Type?}

D -->|int| E[readU32]
D -->|string| F[readString]
D -->|vector| G[readVector]

E --> H[Assign via Pointer]
F --> H
G --> H

H --> I[Database Struct]
```


---
## Database and Filter Flow
```mermaid
flowchart LR

A[Database] -->|getFilteredIDs| B[filterProcessor]

B --> C[Loop Records]

C --> D[Loop Fields]

D --> E[Get Field via Metadata]

E --> F{Field Type?}

F -->|Scalar| G[Range Check]
F -->|Vector| H[Include/Exclude Check]

G --> I{Pass?}
H --> I

I -->|Fail| J[Skip Record]
I -->|Pass| K[Continue Fields]

K --> L{All Fields Pass?}

L -->|Yes| M[Add ID to Result]
L -->|No| C

M --> C

C --> N[\Return ID Vector\]
```
---
## Meta-Data System
```mermaid
flowchart LR

A[FieldMeta] --> B[Schema Generation]
A --> C[Serialization]
A --> D[Deserialization]
A --> E[Filtering]

B --> F[schemaData]
C --> G[ByteWriter]
D --> H[ByteReader]
E --> I[filterProcessor]
```
---
## Dependancies
```mermaid
flowchart LR

animeDB --> fileHandler
animeDB --> filterProcessor
animeDB --> animeData
animeDB --> schemaData

filterProcessor --> schemaData
filterProcessor --> recordField

fileHandler --> schemaData
fileHandler --> recordField

animeSerializer --> animeData
animeSerializer --> ByteWriter
animeSerializer --> ByteReader
animeSerializer --> FieldMeta

networkLayer --> MessageParser
networkLayer --> MessageBuilder

MessageParser --> ByteReader
MessageBuilder --> ByteWriter

RequestHandler --> animeDB
RequestHandler --> animeSerializer
```

---
## Architecture Layers
```mermaid
flowchart LR

A[Client] --> B[Network Layer]
B --> C[Message Layer]
C --> D[Request Handler]

D --> E[Database Layer]
E --> F[Filter Layer]

D --> G[Serialization Layer]
G --> C
```