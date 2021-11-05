const mongoose = require('mongoose');
const Schema = mongoose.Schema;

const userSchema = new Schema({
  firstname: { type: String},
  lastname: { type: String },
   balance: { type: String},
    email: { type: String},
   card: { type: String, required:true },
});

const client = mongoose.model('client',userSchema);

module.exports = client