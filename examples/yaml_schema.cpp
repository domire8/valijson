/**
 * @file
 *
 * @brief Demonstrates validation against a schema loaded from a file.
 *
 */

#include <iostream>
#include <stdexcept>

#include <valijson/adapters/nlohmann_json_adapter.hpp>
#include <valijson/adapters/yaml_cpp_adapter.hpp>
#include <valijson/utils/nlohmann_json_utils.hpp>
#include <valijson/schema.hpp>
#include <valijson/schema_parser.hpp>
#include <valijson/validation_results.hpp>
#include <valijson/validator.hpp>

using std::cerr;
using std::endl;

using valijson::Schema;
using valijson::SchemaParser;
using valijson::Validator;
using valijson::ValidationResults;
using valijson::adapters::NlohmannJsonAdapter;
using valijson::adapters::YamlCppAdapter;

int main(int argc, char *argv[])
{
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " <schema document> <test/target document>" << endl;
        return 1;
    }

    // Load the document containing the schema
    nlohmann::json schemaDocument;
    if (!valijson::utils::loadDocument(argv[1], schemaDocument)) {
        cerr << "Failed to load schema document." << endl;
        return 1;
    }

    // Load the document that is to be validated
    YAML::Node targetDocument;
    try {
        targetDocument = YAML::LoadFile(argv[2]);
    } catch (const YAML::ParserException& ex) {
        cerr << ex.what() << endl;
        return 1;
    }

    // Parse the json schema into an internal schema format
    Schema schema;
    SchemaParser parser;
    NlohmannJsonAdapter schemaDocumentAdapter(schemaDocument);
    try {
        parser.populateSchema(schemaDocumentAdapter, schema);
    } catch (std::exception &e) {
        cerr << "Failed to parse schema: " << e.what() << endl;
        return 1;
    }

    // Perform validation
    Validator validator(Validator::kWeakTypes);
    ValidationResults results;
    YamlCppAdapter targetDocumentAdapter(targetDocument);
    if (!validator.validate(schema, targetDocumentAdapter, &results)) {
        std::cerr << "Validation failed." << endl;
        ValidationResults::Error error;
        unsigned int errorNum = 1;
        while (results.popError(error)) {

            std::string context;
            std::vector<std::string>::iterator itr = error.context.begin();
            for (; itr != error.context.end(); itr++) {
                context += *itr;
            }

            cerr << "Error #" << errorNum << std::endl
                 << "  context: " << context << endl
                 << "  desc:    " << error.description << endl;
            ++errorNum;
        }
        return 1;
    }

    return 0;
}
