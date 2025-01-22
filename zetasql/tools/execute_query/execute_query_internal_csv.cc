//
// Copyright 2019 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include <fcntl.h>

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <algorithm>

#include "zetasql/base/logging.h"
#include "google/protobuf/descriptor.h"
#include "zetasql/analyzer/resolver.h"
#include "zetasql/parser/parser.h"
#include "zetasql/public/analyzer.h"
#include "zetasql/public/analyzer_options.h"
#include "zetasql/public/analyzer_output.h"
#include "zetasql/public/analyzer_output_properties.h"
#include "zetasql/public/catalog.h"
#include "zetasql/public/numeric_value.h"
#include "zetasql/public/simple_catalog.h"
#include "zetasql/public/types/type.h"
#include "zetasql/public/types/type_factory.h"
#include "zetasql/public/value.h"
#include "zetasql/public/cast.h"
#include "zetasql/public/functions/cast_date_time.h"
#include "zetasql/public/functions/convert.h"
#include "zetasql/public/functions/convert_proto.h"
#include "zetasql/public/functions/convert_string.h"
#include "zetasql/public/functions/convert_string_with_format.h"
#include "zetasql/public/functions/date_time_util.h"
#include "zetasql/public/functions/datetime.pb.h"
#include "zetasql/public/functions/range.h"
#include "zetasql/public/functions/string.h"

#include "zetasql/public/simple_catalog.h"
#include "absl/memory/memory.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "absl/strings/str_split.h"
#include "absl/types/optional.h"
#include "riegeli/bytes/fd_reader.h"
#include "riegeli/csv/csv_reader.h"
#include "zetasql/base/status_macros.h"

namespace zetasql {

// Based on TestCastExpression from resolver_test.cc
// Value CastValueToType(std::string& value, const Type* cast_type) {
//   std::string type_name = cast_type->ShortTypeName(ProductMode::PRODUCT_INTERNAL);
//   std::string cast_expression = "CAST('" + value + "' AS " + type_name + ")";
//   std::string error_expression = "Failed to parse " + type_name + " value: " + value;
  
//   // TO DO: Handle errors
//   std::unique_ptr<ParserOutput> parser_output;
//   std::unique_ptr<const ResolvedExpr> resolved_expression;
//   ParseExpression(cast_expression, ParserOptions(), &parser_output);
//   const ASTExpression* parsed_expression = parser_output->expression();
//   ResolveExpr(parsed_expression, &resolved_expression);
//   Value result;
//   result.CopyFrom(resolved_expression.get());
//   return result;
// }

absl::StatusOr<std::unique_ptr<SimpleTable>> MakeTableFromCsvFile(
    absl::string_view table_name, absl::string_view path) {
  riegeli::CsvReader csv_reader{riegeli::FdReader(path)};

  std::vector<std::string> record;
  if (!csv_reader.ReadRecord(record)) {
    if (!csv_reader.ok()) return csv_reader.status();
    return zetasql_base::UnknownErrorBuilder()
           << "CSV file " << path << " does not contain a header row";
  }
  std::vector<SimpleTable::NameAndType> columns;
  columns.reserve(record.size());
  functions::TimestampScale scale = functions::kMicroseconds;

  //Resolver resolver(nullptr, new TypeFactory(TypeFactoryOptions()), new AnalyzerOptions());

  for (const std::string& column_name : record) {
    // Identify type
    std::vector<std::string> parts = absl::StrSplit(column_name, ':');

    // If there is no type, default to string
    if (parts.size() != 2){
      columns.emplace_back(column_name, types::StringType());
      continue;
    }

    // Explicitly supported types
    /*
    const Type* Int32Type() { return s_int32_type(); }
    const Type* Int64Type() { return s_int64_type(); }
    const Type* Uint32Type() { return s_uint32_type(); }
    const Type* Uint64Type() { return s_uint64_type(); }
    const Type* BoolType() { return s_bool_type(); }
    const Type* FloatType() { return s_float_type(); }
    const Type* DoubleType() { return s_double_type(); }
    const Type* StringType() { return s_string_type(); }
    const Type* DateType() { return s_date_type(); }
    const Type* TimestampType() { return s_timestamp_type(); }
    const Type* DatetimeType() { return s_datetime_type(); }
    */

    const std::string& name = parts[0];
    std::string& type_str = parts[1];
    std::transform(type_str.begin(), type_str.end(), type_str.begin(), ::toupper);
    const Type* type;
    if (type_str == "STRING") {
      type = types::StringType();
    } else if (type_str == "INT64") {
      type = types::Int64Type();
    } else if (type_str == "INT32") {
      type = types::Int32Type();
    } else if (type_str == "UINT64") {
      type = types::Uint64Type();
    } else if (type_str == "UINT32") {
      type = types::Uint32Type();
    } else if (type_str == "FLOAT") {
      type = types::FloatType();
    } else if (type_str == "DOUBLE") {
      type = types::DoubleType();
    } else if (type_str == "BOOL") {
      type = types::BoolType();
    } else if (type_str == "TIME") {
      type = types::TimeType();
    } else if (type_str == "DATE") {
      type = types::DateType();
    } else if (type_str == "TIMESTAMP") {
      type = types::TimestampType();
    } else if (type_str == "DATETIME") {
      type = types::DatetimeType();
    } else {
      /*
      // Fall back to parsing the type
      std::unique_ptr<ParserOutput> parser_output;
      absl::Status parser_status, resolver_status;
      parser_status = ParseType(type_str, ParserOptions(), &parser_output);
      if (!parser_status.ok()) {
        resolver_status = resolver.ResolveType(parser_output->type(), {.allow_type_parameters = true, .allow_collation = true}, type, nullptr);
      }
      
      if (!parser_status.ok() || !resolver_status.ok()) {
        return zetasql_base::UnknownErrorBuilder() << "Unsupported/failed to parse type: " << type_str;
      }
      */
      return zetasql_base::UnknownErrorBuilder() << "Unsupported type: " << type_str;
    }
    std::cout << "Parsed type: " << type_str << " to " << type->ShortTypeName(ProductMode::PRODUCT_INTERNAL) << std::endl;
    columns.emplace_back(name, type);
  }

  std::vector<std::vector<Value>> contents;
  while (csv_reader.ReadRecord(record)) {
    if (record.size() != columns.size()) {
      return zetasql_base::UnknownErrorBuilder()
             << "CSV file " << path << " has a header row with "
             << columns.size() << " columns, but row "
             << csv_reader.last_record_index() << " has " << record.size()
             << " fields";
    }
    std::vector<Value>& row = contents.emplace_back();
    row.reserve(record.size());
    for (int i = 0; i < record.size(); ++i) {
      const std::string& field = record[i];
      const Type* type = columns[i].second;

      if (type == types::StringType()) {
        row.push_back(Value::String(field));

      // Handle basic types natively for performance
      } else if (type == types::Int64Type()) {
        int64_t int_value;
        if (!absl::SimpleAtoi(field, &int_value)) {
          return zetasql_base::UnknownErrorBuilder()
            << "Failed to parse INT64 value: " << field;
        }
        row.push_back(Value::Int64(int_value));
      } else if (type == types::Int32Type()) {
        int32_t int_value;
        if (!absl::SimpleAtoi(field, &int_value)) {
          return zetasql_base::UnknownErrorBuilder()
            << "Failed to parse INT32 value: " << field;
        }
        row.push_back(Value::Int32(int_value));
      } else if (type == types::Uint64Type()) {
        uint64_t uint_value;
        if (!absl::SimpleAtoi(field, &uint_value)) {
          return zetasql_base::UnknownErrorBuilder()
            << "Failed to parse UINT64 value: " << field;
        }
        row.push_back(Value::Uint64(uint_value));
      } else if (type == types::Uint32Type()) {
        uint32_t uint_value;
        if (!absl::SimpleAtoi(field, &uint_value)) {
          return zetasql_base::UnknownErrorBuilder()
            << "Failed to parse UINT32 value: " << field;
        }
        row.push_back(Value::Uint32(uint_value));
      } else if (type == types::FloatType()) {
        float float_value;
        if (!absl::SimpleAtof(field, &float_value)) {
          return zetasql_base::UnknownErrorBuilder()
            << "Failed to parse FLOAT value: " << field;
        }
        row.push_back(Value::Float(float_value));
      } else if (type == types::DoubleType()) {
        double double_value;
        if (!absl::SimpleAtod(field, &double_value)) {
          return zetasql_base::UnknownErrorBuilder()
            << "Failed to parse DOUBLE value: " << field;
        }
        row.push_back(Value::Double(double_value));
      } else if (type == types::BoolType()) {
        bool bool_value;
        if (!absl::SimpleAtob(field, &bool_value)) {
          return zetasql_base::UnknownErrorBuilder()
            << "Failed to parse BOOL value: " << field;
        }
        row.push_back(Value::Bool(bool_value));
      } else if (type == types::TimeType()) {
        TimeValue time;
        absl::Status status = functions::ConvertStringToTime(field, scale, &time);
        if (!status.ok()) {
          return zetasql_base::UnknownErrorBuilder()
            << "Failed to parse TIME value: " << field;
        }
        row.push_back(Value::Time(time));
      } else if (type == types::DatetimeType()) {
        DatetimeValue datetime;
        absl::Status status = functions::ConvertStringToDatetime(field, scale, &datetime);
        if (!status.ok()) {
          return zetasql_base::UnknownErrorBuilder()
            << "Failed to parse DATETIME value: " << field;
        }
        row.push_back(Value::Datetime(datetime));
      } else if (type == types::TimestampType()) {
        absl::Time timestamp;
        absl::Status status = functions::ConvertStringToTimestamp(field, absl::UTCTimeZone(), scale, true /* allow_tz_in_str */, &timestamp);
        if (!status.ok()) {
          return zetasql_base::UnknownErrorBuilder()
            << "Failed to parse TIMESTAMP value: " << field;
        }
        row.push_back(Value::Timestamp(timestamp));
      } else if (type == types::DateType()) {
        int32_t date;
        absl::Status status = functions::ConvertStringToDate(field, &date);
        if (!status.ok()) {
          return zetasql_base::UnknownErrorBuilder()
            << "Failed to parse DATE value: " << field;
        }
        row.push_back(Value::Date(date));
      } else {
        return zetasql_base::UnknownErrorBuilder()
            << "Error casting " << field << " to " << type->DebugString();
          /*
          // Fall back using the cast expression
          std::string type_name = type->ShortTypeName(ProductMode::PRODUCT_INTERNAL);
          std::string cast_expression = "CAST('" + field + "' AS " + type_name + ")";
    
          std::unique_ptr<ParserOutput> parser_output;
          std::unique_ptr<const ResolvedExpr> resolved_expression;
          absl::Status parser_status, resolver_status;
          parser_status = ParseExpression(cast_expression, ParserOptions(), &parser_output);
          resolver_status = resolver.ResolveExpr(parser_output->expression(), &resolved_expression);
          if (!parser_status.ok() || !resolver_status.ok()) {    

          }
          Value any_value;
          any_value.copy_from(resolved_expression.get());
          row.push_back(any_value);
          */
      }
    }
  }
  if (!csv_reader.Close()) return csv_reader.status();

  auto table = std::make_unique<SimpleTable>(table_name, columns);
  table->SetContents(contents);
  return table;
}

}  // namespace zetasql
