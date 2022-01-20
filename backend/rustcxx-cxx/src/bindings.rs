use serde::Deserialize;
use std::collections::HashMap;
use crate::function::Function;
use crate::datatype::TypeName;

#[derive(Debug, Clone, Deserialize)]
pub struct TypeBindings {
    pub methods: HashMap<String, Function>,
}

#[derive(Debug, Clone, Deserialize)]
pub struct Bindings(pub HashMap<TypeName, TypeBindings>);
