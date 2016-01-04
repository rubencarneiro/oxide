// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2015 Canonical Ltd.

// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#include "oxide_qt_variant_value_converter.h"

#include <utility>

#include <QList>
#include <QMap>
#include <QString>
#include <QVariant>

#include "base/logging.h"
#include "base/values.h"

namespace oxide {
namespace qt {

namespace {
const int kMaxRecursionDepth = 100;
}

class State {
 public:
  State() : recursion_depth_(0) {}
  ~State() { DCHECK_EQ(recursion_depth_, 0); }

  bool HasReachedMaxRecursionDepth() const {
    return recursion_depth_ >= kMaxRecursionDepth;
  }

  class Scope {
   public:
    Scope(State* state) : state_(state) { ++state_->recursion_depth_; }
    ~Scope() { --state_->recursion_depth_; }

   private:
    State* state_;
  };

 private:
  int recursion_depth_;
};

namespace {

scoped_ptr<base::Value> FromVariantValueInternal(const QVariant& variant,
                                                 State* state);

scoped_ptr<base::ListValue> FromVariantListValueInternal(
    const QList<QVariant>& list,
    State* state) {
  scoped_ptr<base::ListValue> rv(new base::ListValue());

  for (auto it = list.begin(); it != list.end(); ++it) {
    scoped_ptr<base::Value> value = FromVariantValueInternal(*it, state);
    if (!value) {
      value = base::Value::CreateNullValue();
    }
    rv->Append(std::move(value));
  }

  return std::move(rv);
}

scoped_ptr<base::DictionaryValue> FromVariantMapValueInternal(
    const QMap<QString, QVariant>& map,
    State* state) {
  scoped_ptr<base::DictionaryValue> rv(new base::DictionaryValue());

  for (auto it = map.begin(); it != map.end(); ++it) {
    scoped_ptr<base::Value> value = FromVariantValueInternal(*it, state);
    if (!value) {
      continue;
    }
    rv->Set(it.key().toStdString(), std::move(value));
  }

  return std::move(rv);
}

scoped_ptr<base::Value> FromVariantValueInternal(const QVariant& variant,
                                                 State* state) {
  State::Scope scope(state);

  if (state->HasReachedMaxRecursionDepth()) {
    return nullptr;
  }

  switch (variant.type()) {
    case QVariant::Bool:
      return make_scoped_ptr(new base::FundamentalValue(variant.toBool()));
    case QVariant::Double:
    case QVariant::LongLong:
    case QVariant::UInt:
    case QVariant::ULongLong:
      return make_scoped_ptr(new base::FundamentalValue(variant.toDouble()));
    case QVariant::Int:
      return make_scoped_ptr(new base::FundamentalValue(variant.toInt()));
    case QVariant::List:
    case QVariant::StringList:
      return FromVariantListValueInternal(variant.toList(), state);
    case QVariant::Map:
      return FromVariantMapValueInternal(variant.toMap(), state);
    case QVariant::String:
      return make_scoped_ptr(
          new base::StringValue(variant.toString().toStdString()));
    default:
      break;
  }

  if (variant.isNull()) {
    return base::Value::CreateNullValue();
  }

  QString str = variant.toString();
  if (str.isEmpty()) {
    return nullptr;
  }

  return make_scoped_ptr(
      new base::StringValue(variant.toString().toStdString()));
}

QVariant ToVariantValueInternal(const base::Value* value,
                                State* state,
                                bool* success);

QList<QVariant> ToVariantListValueInternal(const base::ListValue* list,
                                           State* state) {
  QList<QVariant> rv;

  for (auto it = list->begin(); it != list->end(); ++it) {
    bool dummy;
    rv.push_back(ToVariantValueInternal(*it, state, &dummy));
  }

  return rv;
}

QMap<QString, QVariant> ToVariantMapValueInternal(
    const base::DictionaryValue* dict,
    State* state) {
  QMap<QString, QVariant> rv;

  base::DictionaryValue::Iterator iter(*dict);
  while (!iter.IsAtEnd()) {
    bool success;
    QVariant value = ToVariantValueInternal(&iter.value(), state, &success);
    if (success) {
      rv[QString::fromStdString(iter.key())] = value;
    }
    iter.Advance();
  }

  return rv;
}

QVariant ToVariantValueInternal(const base::Value* value,
                                State* state,
                                bool* success) {
  State::Scope scope(state);

  *success = true;

  if (state->HasReachedMaxRecursionDepth()) {
    *success = false;
    return QVariant();
  }

  switch (value->GetType()) {
    case base::Value::TYPE_NULL:
      return QVariant();
    case base::Value::TYPE_BOOLEAN: {
      bool rv;
      value->GetAsBoolean(&rv);
      return rv;
    }
    case base::Value::TYPE_INTEGER: {
      int rv;
      value->GetAsInteger(&rv);
      return rv;
    }
    case base::Value::TYPE_DOUBLE: {
      double rv;
      value->GetAsDouble(&rv);
      return rv;
    }
    case base::Value::TYPE_STRING: {
      std::string rv;
      value->GetAsString(&rv);
      return QString::fromStdString(rv);
    }
    case base::Value::TYPE_BINARY: {
      *success = false;
      return QVariant();
    }
    case base::Value::TYPE_DICTIONARY: {
      const base::DictionaryValue* dict;
      value->GetAsDictionary(&dict);
      return ToVariantMapValueInternal(dict, state);
    }
    case base::Value::TYPE_LIST: {
      const base::ListValue* list;
      value->GetAsList(&list);
      return ToVariantListValueInternal(list, state);
    }
    default:
      NOTREACHED();
      *success = false;
      return QVariant();
  }
}

}

// static
scoped_ptr<base::Value> VariantValueConverter::FromVariantValue(
    const QVariant& variant) {
  State state;
  return FromVariantValueInternal(variant, &state);
}

// static
QVariant VariantValueConverter::ToVariantValue(const base::Value* value) {
  State state;
  bool dummy;
  return ToVariantValueInternal(value, &state, &dummy);
}

} // namespace qt
} // namespace oxide
