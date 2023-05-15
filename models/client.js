const mongoose = require('mongoose');
const Schema = mongoose.Schema;

const userSchema = new Schema({
 firstname: { type: String},
  lastname: { type: String},
  NID: { type: String},
  District: { type: String},
  Sector: { type: String},
  Cell: { type: String},
  Village: { type: String},
  Phone: { type: String},
  card: { type: String, required:true },
});

const client = mongoose.model('client',userSchema);

module.exports = client