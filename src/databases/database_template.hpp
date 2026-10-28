#pragma once






template <typename Id,typename Record>
class Database {
	private:
		std::unordered_map<Id, size_t> idMap;
		Id nextId;

		std::vector<Record> database;
		std::vector<size_t> fieldMap;

	public:
	    // Main File I/O
		void load();
		void save();

		// Read Handlers
		void section(const sectionHeader& header);
		void schema(const sectionHeader& section, const schemaData& schema);
		void record(const sectionHeader& section, const recordData& record, const schemaData& schema);
		void table(const sectionHeader& section, const std::string& table);

		// Write Handlers
		fileHeader    getHeader(fileType type) const;
		sectionHeader getSection(fileType type, size_t index) const;
		sectionHeader getSection() const;
		schemaData    getSchema(fileType type, size_t index) const;
		schemaData    getSchema() const;
		recordData    getRecord(fileType type, size_t schemaIndex, size_t recordIndex) const;
		recordData    getTableRecord(fileType type, size_t sectionIndex, size_t rowIndex);
		Id            getId(size_t index) const;
		size_t        getFieldIndex(const std::string& fieldName) const;

		// Database Mutations
		void add(); // Takse a singular or vector of records
	    void update(); // Update input
	    void deleteRecords(); // Singular or range input of ids

		// Database Queries
		bool contains(); // takes a id
	    void get(); // Singular or range input of ids
		void filter(); // Needs the filters
	    void search(); // Needs the search query, use string for now

		// Field Mutations
		void setField(); // Need to take a id or a reference to a record as input as well as field index, and needs the value
	    void addtoField(); // Need to take a id or a reference to a record as input as well as field index, and needs the value
	    void removeFieldValue(); // Need to take a id or a reference to a record as input as well as field index, and needs the value
	    void clearField(); // Need to take a id or a reference to a record as input as well as field index

		// Field Queries
		void getField(); //Need to take a id or a reference to a record as input, also a field index or title
		void getFieldIndex(); // returns the field index using its title

		// Validation Handlers
		void validateIds();
		void validateRanks();
		void validateTitles();
		void validateRanges();
		void validateReferences();
		template <typename T> void validateExternal(const T& external);
}


