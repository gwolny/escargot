#include "Escargot.h"
#include "Object.h"
#include "ExecutionContext.h"
#include "Context.h"
#include "ErrorObject.h"
#include "FunctionObject.h"
#include "ArrayObject.h"

namespace Escargot {

Value ObjectGetResult::valueSlowCase(ExecutionState& state) const
{
    if (m_accessorData.m_jsGetterSetter->getter()) {
        ASSERT(m_accessorData.m_self);
        return m_accessorData.m_jsGetterSetter->getter()->call(state, m_accessorData.m_self, 0, nullptr);
    }
    return Value();
}

ObjectPropertyDescriptor::ObjectPropertyDescriptor(ExecutionState& state, Object* obj)
    : m_isDataProperty(true)
{
    m_property = NotPresent;
    const StaticStrings* strings = &state.context()->staticStrings();
    auto desc = obj->get(state, ObjectPropertyName(strings->enumerable));
    if (desc.hasValue())
        setEnumerable(desc.value(state).toBoolean(state));
    desc = obj->get(state, ObjectPropertyName(strings->configurable));
    if (desc.hasValue())
        setConfigurable(desc.value(state).toBoolean(state));

    bool hasWritable = false;
    desc = obj->get(state, ObjectPropertyName(strings->writable));
    if (desc.hasValue()) {
        setWritable(desc.value(state).toBoolean(state));
        hasWritable = true;
    }

    bool hasValue = false;
    desc = obj->get(state, ObjectPropertyName(strings->value));
    if (desc.hasValue()) {
        m_value = desc.value(state);
        hasValue = true;
    }

    desc = obj->get(state, ObjectPropertyName(strings->get));
    if (desc.hasValue()) {
        Value v = desc.value(state);
        if (!v.isFunction()) {
            ErrorObject::throwBuiltinError(state, ErrorObject::TypeError, "Getter must be a function");
        } else {
            m_isDataProperty = false;
            m_getterSetter = JSGetterSetter(v.asFunction(), nullptr);
        }
    }

    desc = obj->get(state, ObjectPropertyName(strings->set));
    if (desc.hasValue()) {
        Value v = desc.value(state);
        if (!v.isFunction()) {
            ErrorObject::throwBuiltinError(state, ErrorObject::TypeError, "Setter must be a function");
        } else {
            if (m_isDataProperty) {
                m_isDataProperty = false;
                m_getterSetter = JSGetterSetter(v.asFunction(), nullptr);
            } else {
                m_getterSetter.m_setter = v.asFunction();
            }
        }
    }

    if (!m_isDataProperty) {
        if (hasWritable | hasValue) {
            ErrorObject::throwBuiltinError(state, ErrorObject::TypeError, "Invalid property descriptor. Cannot both specify accessors and a value or writable attribute");
        }
    }

    checkProperty();
}

void ObjectPropertyDescriptor::setEnumerable(bool enumerable)
{
    if (enumerable)
        m_property = (PresentAttribute)(m_property | EnumerablePresent);
    else
        m_property = (PresentAttribute)(m_property | NonEnumerablePresent);
}

void ObjectPropertyDescriptor::setConfigurable(bool configurable)
{
    if (configurable)
        m_property = (PresentAttribute)(m_property | ConfigurablePresent);
    else
        m_property = (PresentAttribute)(m_property | NonConfigurablePresent);
}

void ObjectPropertyDescriptor::setWritable(bool writable)
{
    if (writable)
        m_property = (PresentAttribute)(m_property | WritablePresent);
    else
        m_property = (PresentAttribute)(m_property | NonWritablePresent);
}

ObjectStructurePropertyDescriptor ObjectPropertyDescriptor::toObjectStructurePropertyDescriptor() const
{
    if (isDataProperty()) {
        int f = 0;

        if (isWritable()) {
            f = ObjectStructurePropertyDescriptor::WritablePresent;
        }

        if (isConfigurable()) {
            f |= ObjectStructurePropertyDescriptor::ConfigurablePresent;
        }

        if (isEnumerable()) {
            f |= ObjectStructurePropertyDescriptor::EnumerablePresent;
        }

        return ObjectStructurePropertyDescriptor::createDataDescriptor((ObjectStructurePropertyDescriptor::PresentAttribute)f);
    } else {
        int f = 0;

        if (isConfigurable()) {
            f |= ObjectStructurePropertyDescriptor::ConfigurablePresent;
        }

        if (isEnumerable()) {
            f |= ObjectStructurePropertyDescriptor::EnumerablePresent;
        }

        if (hasJSGetter()) {
            f |= ObjectStructurePropertyDescriptor::HasJSGetter;
        }

        if (hasJSSetter()) {
            f |= ObjectStructurePropertyDescriptor::HasJSSetter;
        }

        return ObjectStructurePropertyDescriptor::createAccessorDescriptor((ObjectStructurePropertyDescriptor::PresentAttribute)f);
    }
}

Object::Object(ExecutionState& state, size_t defaultSpace, bool initPlainArea)
    : m_structure(state.context()->defaultStructureForObject())
    , m_rareData(nullptr)
{
    m_values.resizeWithUninitializedValues(defaultSpace);
    if (initPlainArea) {
        initPlainObject(state);
    }
}

Object::Object(ExecutionState& state)
    : Object(state, ESCARGOT_OBJECT_BUILTIN_PROPERTY_NUMBER, true)
{
}

void Object::initPlainObject(ExecutionState& state)
{
    m_values[0] = Value(state.context()->globalObject()->objectPrototype());
}

Object* Object::createBuiltinObjectPrototype(ExecutionState& state)
{
    Object* obj = new Object(state, 1, false);
    obj->m_structure = state.context()->defaultStructureForObject();
    obj->m_values[0] = Value(Value::Null);
    return obj;
}

Object* Object::createFunctionPrototypeObject(ExecutionState& state, FunctionObject* function)
{
    Object* obj = new Object(state, 2, false);
    obj->m_structure = state.context()->defaultStructureForFunctionPrototypeObject();
    obj->m_values[0] = Value(state.context()->globalObject()->objectPrototype());
    obj->m_values[1] = Value(function);

    return obj;
}

Value Object::getPrototypeSlowCase(ExecutionState& state)
{
    return getOwnProperty(state, ObjectPropertyName(state.context()->staticStrings().__proto__)).value(state);
}

bool Object::setPrototypeSlowCase(ExecutionState& state, const Value& value)
{
    return defineOwnProperty(state, ObjectPropertyName(state, state.context()->staticStrings().__proto__), ObjectPropertyDescriptor(value));
}

// http://www.ecma-international.org/ecma-262/6.0/#sec-ordinarygetownproperty
ObjectGetResult Object::getOwnProperty(ExecutionState& state, const ObjectPropertyName& propertyName) ESCARGOT_OBJECT_SUBCLASS_MUST_REDEFINE
{
    if (propertyName.isUIntType() && !m_structure->hasIndexPropertyName()) {
        return ObjectGetResult();
    }

    PropertyName P = propertyName.toPropertyName(state);
    size_t idx = m_structure->findProperty(state, P);
    if (LIKELY(idx != SIZE_MAX)) {
        const ObjectStructureItem& item = m_structure->readProperty(state, idx);
        if (item.m_descriptor.isDataProperty()) {
            if (LIKELY(!item.m_descriptor.isNativeAccessorProperty())) {
                return ObjectGetResult(m_values[idx], item.m_descriptor.isWritable(), item.m_descriptor.isEnumerable(), item.m_descriptor.isConfigurable());
            } else {
                ObjectPropertyNativeGetterSetterData* data = item.m_descriptor.nativeGetterSetterData();
                return ObjectGetResult(data->m_getter(state, this), item.m_descriptor.isWritable(), item.m_descriptor.isEnumerable(), item.m_descriptor.isConfigurable());
            }
        } else {
            Value v = m_values[idx];
            ASSERT(v.isPointerValue() && v.asPointerValue()->isJSGetterSetter());
            return ObjectGetResult(this, v, item.m_descriptor.isEnumerable(), item.m_descriptor.isConfigurable());
        }
    }
    return ObjectGetResult();
}

bool Object::defineOwnProperty(ExecutionState& state, const ObjectPropertyName& P, const ObjectPropertyDescriptor& desc) ESCARGOT_OBJECT_SUBCLASS_MUST_REDEFINE
{
    if (isEverSetAsPrototypeObject()) {
        if (P.toValue(state).toIndex(state) != Value::InvalidIndexValue) {
            // TODO
            // implement bad time
            RELEASE_ASSERT_NOT_REACHED();
        }
    }

    // TODO Return true, if every field in Desc is absent.
    // TODO Return true, if every field in Desc also occurs in current and the value of every field in Desc is the same value as the corresponding field in current when compared using the SameValue algorithm (9.12).

    PropertyName propertyName = P.toPropertyName(state);
    size_t oldIdx = m_structure->findProperty(state, propertyName);
    if (oldIdx == SIZE_MAX) {
        // 3. If current is undefined and extensible is false, then Reject.
        if (UNLIKELY(!isExtensible()))
            return false;

        m_structure = m_structure->addProperty(state, propertyName, desc.toObjectStructurePropertyDescriptor());
        if (LIKELY(desc.isDataProperty())) {
            m_values.pushBack(desc.value());
        } else {
            m_values.pushBack(Value(new JSGetterSetter(desc.getterSetter())));
        }

        ASSERT(m_values.size() == m_structure->propertyCount());
        return true;
    } else {
        size_t idx = oldIdx;
        const ObjectStructureItem& item = m_structure->readProperty(state, idx);

        // If the [[Configurable]] field of current is false then
        if (!item.m_descriptor.isConfigurable()) {
            // Reject, if the [[Configurable]] field of Desc is true.
            if (desc.isConfigurable()) {
                return false;
            }
            // Reject, if the [[Enumerable]] field of Desc is present and the [[Enumerable]] fields of current and Desc are the Boolean negation of each other.
            if (desc.isEnumerablePresent() && desc.isEnumerable() != item.m_descriptor.isEnumerable()) {
                return false;
            }
        }

        // TODO If IsGenericDescriptor(Desc) is true, then no further validation is required.
        // if IsDataDescriptor(current) and IsDataDescriptor(Desc) have different results, then
        if (item.m_descriptor.isDataProperty() != desc.isDataProperty()) {
            // Reject, if the [[Configurable]] field of current is false.
            if (!item.m_descriptor.isConfigurable()) {
                return false;
            }

            auto current = item.m_descriptor;
            deleteOwnProperty(state, P);

            int f = 0;
            if (current.isConfigurable()) {
                f = f | ObjectPropertyDescriptor::ConfigurablePresent;
            } else {
                f = f | ObjectPropertyDescriptor::NonConfigurablePresent;
            }

            if (current.isEnumerable()) {
                f = f | ObjectPropertyDescriptor::EnumerablePresent;
            } else {
                f = f | ObjectPropertyDescriptor::NonEnumerablePresent;
            }

            // If IsDataDescriptor(current) is true, then
            if (current.isDataProperty()) {
                // Convert the property named P of object O from a data property to an accessor property.
                // Preserve the existing values of the converted property’s [[Configurable]] and [[Enumerable]] attributes
                // and set the rest of the property’s attributes to their default values.
                return defineOwnProperty(state, P, ObjectPropertyDescriptor(desc.getterSetter(), (ObjectPropertyDescriptor::PresentAttribute)f));
            } else {
                // Else,
                // Convert the property named P of object O from a data property to an accessor property.
                // Preserve the existing values of the converted property’s [[Configurable]] and [[Enumerable]] attributes
                // and set the rest of the property’s attributes to their default values.
                if (current.isWritable()) {
                    f = f | ObjectPropertyDescriptor::WritablePresent;
                } else {
                    f = f | ObjectPropertyDescriptor::NonWritablePresent;
                }
                return defineOwnProperty(state, P, ObjectPropertyDescriptor(desc.value(), (ObjectPropertyDescriptor::PresentAttribute)f));
            }
        }
        // Else, if IsDataDescriptor(current) and IsDataDescriptor(Desc) are both true, then
        else if (item.m_descriptor.isDataProperty() && desc.isDataProperty()) {
            // If the [[Configurable]] field of current is false, then
            if (!item.m_descriptor.isConfigurable()) {
                // Reject, if the [[Writable]] field of current is false and the [[Writable]] field of Desc is true.
                if (item.m_descriptor.isConfigurable() && !desc.isWritable()) {
                    return false;
                }
                // If the [[Writable]] field of current is false, then
                if (!item.m_descriptor.isWritable()) {
                    // Reject, if the [[Value]] field of Desc is present and SameValue(Desc.[[Value]], current.[[Value]]) is false.
                    if (!desc.value().equalsTo(state, getOwnDataPropertyUtilForObject(state, idx, this))) {
                        return false;
                    }
                }
            }
            // else, the [[Configurable]] field of current is true, so any change is acceptable.
        } else {
            // Else, IsAccessorDescriptor(current) and IsAccessorDescriptor(Desc) are both true so,

            // If the [[Configurable]] field of current is false, then
            if (!item.m_descriptor.isConfigurable()) {
                Value c = m_values[idx];

                // Reject, if the [[Set]] field of Desc is present and SameValue(Desc.[[Set]], current.[[Set]]) is false.
                if (c.asPointerValue()->asJSGetterSetter()->getter() != desc.getterSetter().getter()) {
                    return false;
                }

                // Reject, if the [[Get]] field of Desc is present and SameValue(Desc.[[Get]], current.[[Get]]) is false.
                if (c.asPointerValue()->asJSGetterSetter()->setter() != desc.getterSetter().setter()) {
                    return false;
                }
            }
        }
        // For each attribute field of Desc that is present, set the correspondingly named attribute of the property named P of object O to the value of the field.
        bool shouldDelete = false;

        if (desc.isWritablePresent()) {
            if (desc.isWritable() != item.m_descriptor.isWritable()) {
                shouldDelete = true;
            }
        }

        if (!shouldDelete && desc.isEnumerablePresent()) {
            if (desc.isEnumerable() != item.m_descriptor.isEnumerable()) {
                shouldDelete = true;
            }
        }

        if (!shouldDelete && desc.isConfigurablePresent()) {
            if (desc.isConfigurable() != item.m_descriptor.isConfigurable()) {
                shouldDelete = true;
            }
        }

        if (!shouldDelete) {
            if (item.m_descriptor.isDataProperty()) {
                if (LIKELY(!item.m_descriptor.isNativeAccessorProperty())) {
                    m_values[idx] = desc.value();
                    return true;
                } else {
                    ObjectPropertyNativeGetterSetterData* data = item.m_descriptor.nativeGetterSetterData();
                    return data->m_setter(state, this, desc.value());
                }
            } else {
                m_values[idx] = Value(new JSGetterSetter(desc.getterSetter()));
            }
        } else {
            deleteOwnProperty(state, P);
            defineOwnProperty(state, P, desc);
        }

        return true;
    }
}

bool Object::deleteOwnProperty(ExecutionState& state, const ObjectPropertyName& P) ESCARGOT_OBJECT_SUBCLASS_MUST_REDEFINE
{
    auto result = getOwnProperty(state, P);
    if (result.hasValue() && result.isConfigurable()) {
        deleteOwnProperty(state, m_structure->findProperty(state, P.toPropertyName(state)));
        return true;
    }
    return false;
}

void Object::enumeration(ExecutionState& state, std::function<bool(const ObjectPropertyName&, const ObjectStructurePropertyDescriptor& desc)> fn) ESCARGOT_OBJECT_SUBCLASS_MUST_REDEFINE
{
    size_t cnt = m_structure->propertyCount();
    for (size_t i = 0; i < cnt; i++) {
        const ObjectStructureItem& item = m_structure->readProperty(state, i);
        if (!fn(ObjectPropertyName(state, item.m_propertyName), item.m_descriptor)) {
            break;
        }
    }
}

ObjectGetResult Object::get(ExecutionState& state, const ObjectPropertyName& propertyName, Object* receiver)
{
    Object* target = this;
    while (true) {
        auto result = target->getOwnProperty(state, propertyName);
        if (result.hasValue()) {
            return result;
        }
        Value __proto__ = target->getPrototype(state);
        if (__proto__.isObject()) {
            target = __proto__.asObject();
        } else {
            return ObjectGetResult();
        }
    }
}

// http://www.ecma-international.org/ecma-262/6.0/#sec-ordinary-object-internal-methods-and-internal-slots-set-p-v-receiver
bool Object::set(ExecutionState& state, const ObjectPropertyName& propertyName, const Value& v, Object* receiver)
{
    auto desc = getOwnProperty(state, propertyName);
    if (!desc.hasValue()) {
        Value target = this->getPrototype(state);
        while (target.isObject()) {
            Object* O = target.asObject();
            auto desc = O->getOwnProperty(state, propertyName);
            if (desc.hasValue()) {
                return O->set(state, propertyName, v, receiver);
            }
            target = O->getPrototype(state);
        }
        ObjectPropertyDescriptor desc(v, ObjectPropertyDescriptor::AllPresent);
        return defineOwnProperty(state, propertyName, desc);
    } else {
        // If IsDataDescriptor(ownDesc) is true, then
        if (desc.isDataProperty()) {
            // If ownDesc.[[Writable]] is false, return false.
            if (!desc.isWritable()) {
                return false;
            }
            // TODO If Type(Receiver) is not Object, return false.
            // Let existingDescriptor be Receiver.[[GetOwnProperty]](P).
            auto receiverDesc = receiver->getOwnProperty(state, propertyName);
            // If existingDescriptor is not undefined, then
            if (receiverDesc.hasValue()) {
                // If IsAccessorDescriptor(existingDescriptor) is true, return false.
                if (!receiverDesc.isDataProperty()) {
                    return false;
                }
                // If existingDescriptor.[[Writable]] is false, return false
                if (!receiverDesc.isWritable()) {
                    return false;
                }
                // Let valueDesc be the PropertyDescriptor{[[Value]]: V}.
                ObjectPropertyDescriptor desc(v);
                return receiver->defineOwnProperty(state, propertyName, desc);
            } else {
                // Else Receiver does not currently have a property P,
                // Return CreateDataProperty(Receiver, P, V).
                ObjectPropertyDescriptor desc(v);
                return defineOwnProperty(state, propertyName, desc);
            }
        } else {
            // Let setter be ownDesc.[[Set]].
            auto setter = desc.jsGetterSetter()->setter();

            // If setter is undefined, return false.
            if (!setter) {
                return false;
            }

            // Let setterResult be Call(setter, Receiver, «V»).
            Value argv[] = { v };
            setter->call(state, receiver, 1, argv);
            return true;
        }
    }
}

void Object::setThrowsException(ExecutionState& state, const ObjectPropertyName& P, const Value& v, Object* receiver)
{
    if (UNLIKELY(!set(state, P, v, receiver))) {
        ErrorObject::throwBuiltinError(state, ErrorObject::Code::TypeError, P.string(state), false, String::emptyString, errorMessage_DefineProperty_NotWritable);
    }
}

void Object::setThrowsExceptionWhenStrictMode(ExecutionState& state, const ObjectPropertyName& P, const Value& v, Object* receiver)
{
    if (UNLIKELY(!set(state, P, v, receiver)) && state.inStrictMode()) {
        ErrorObject::throwBuiltinError(state, ErrorObject::Code::TypeError, P.string(state), false, String::emptyString, errorMessage_DefineProperty_NotWritable);
    }
}

void Object::throwCannotDefineError(ExecutionState& state, const PropertyName& P)
{
    ErrorObject::throwBuiltinError(state, ErrorObject::Code::TypeError, P.string(), false, String::emptyString, errorMessage_DefineProperty_RedefineNotConfigurable);
}

void Object::throwCannotWriteError(ExecutionState& state, const PropertyName& P)
{
    ErrorObject::throwBuiltinError(state, ErrorObject::Code::TypeError, P.string(), false, String::emptyString, errorMessage_DefineProperty_NotWritable);
}

void Object::deleteOwnProperty(ExecutionState& state, size_t idx)
{
    if (isPlainObject()) {
        const ObjectStructureItem& ownDesc = m_structure->readProperty(state, idx);
        if (ownDesc.m_descriptor.isNativeAccessorProperty()) {
            ensureObjectRareData();
            // TODO modify every native accessor!
            RELEASE_ASSERT_NOT_REACHED();
            m_rareData->m_isPlainObject = false;
        }
    }

    m_structure = m_structure->removeProperty(state, idx);
    m_values.erase(idx);

    ASSERT(m_values.size() == m_structure->propertyCount());
}

uint32_t Object::length(ExecutionState& state)
{
    return get(state, state.context()->staticStrings().length, this).value(state).toUint32(state);
}

double Object::nextIndexForward(ExecutionState& state, Object* obj, const double cur, const double end, const bool skipUndefined)
{
    Value ptr = obj;
    double ret = end;
    while (ptr.isObject()) {
        ptr.asObject()->enumeration(state, [&](const ObjectPropertyName& name, const ObjectStructurePropertyDescriptor& desc) {
            uint32_t index = Value::InvalidArrayIndexValue;
            Value key = name.toValue(state);
            if ((index = key.toArrayIndex(state)) != Value::InvalidArrayIndexValue) {
                if (skipUndefined && ptr.asObject()->get(state, name).value(state).isUndefined()) {
                    return true;
                }
                if (index > cur) {
                    ret = index;
                    return false;
                }
            }
            return true;
        });
        ptr = ptr.asObject()->getPrototype(state);
    }
    return ret;
}

double Object::nextIndexBackward(ExecutionState& state, Object* obj, const double cur, const double end, const bool skipUndefined)
{
    Value ptr = obj;
    double ret = end;
    while (ptr.isObject()) {
        ptr.asObject()->enumeration(state, [&](const ObjectPropertyName& name, const ObjectStructurePropertyDescriptor& desc) {
            uint32_t index = Value::InvalidArrayIndexValue;
            Value key = name.toValue(state);
            if ((index = key.toArrayIndex(state)) != Value::InvalidArrayIndexValue) {
                if (skipUndefined && ptr.asObject()->get(state, name).value(state).isUndefined()) {
                    return true;
                }
                if (index < cur) {
                    ret = index;
                    return false;
                }
            }
            return true;
        });
        ptr = ptr.asObject()->getPrototype(state);
    }
    return ret;
}

void Object::sort(ExecutionState& state, std::function<bool(const Value& a, const Value& b)> comp)
{
    std::vector<Value, gc_malloc_ignore_off_page_allocator<Value>> selected;

    uint32_t len = length(state);
    uint32_t n = 0;
    uint32_t k = 0;

    while (k < len) {
        Value idx = Value(k);
        if (hasOwnProperty(state, ObjectPropertyName(state, idx))) {
            selected.push_back(getOwnProperty(state, ObjectPropertyName(state, idx)).value(state));
            n++;
            k++;
        } else {
            k = nextIndexForward(state, this, k, len, false);
        }
    }

    std::sort(selected.begin(), selected.end(), comp);

    uint32_t i;
    for (i = 0; i < n; i++) {
        setThrowsException(state, ObjectPropertyName(state, Value(i)), selected[i], this);
    }

    while (i < len) {
        Value idx = Value(i);
        if (hasOwnProperty(state, ObjectPropertyName(state, idx))) {
            deleteOwnProperty(state, ObjectPropertyName(state, Value(i)));
            i++;
        } else {
            i = nextIndexForward(state, this, i, len, false);
        }
    }
}
}
